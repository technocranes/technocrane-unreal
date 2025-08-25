// Copyright (c) 2020-2025 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// TechnocraneRig.cpp
// Sergei <Neill3d> Solokhin

#include "TechnocraneRig.h"
#include "TechnocranePrivatePCH.h"
#include <UObject/ConstructorHelpers.h>
#include <Components/SkeletalMeshComponent.h>
#include <Components/PoseableMeshComponent.h>
#include <Engine/SkeletalMesh.h>
#include "Engine/CollisionProfile.h"
#include "DrawDebugHelpers.h"

#include <Runtime/CinematicCamera/Public/CineCameraActor.h>
#include <Runtime/CinematicCamera/Public/CineCameraComponent.h>

#include "Interfaces/IPluginManager.h"

#include "TechnocraneShared.h"
#include <TechnocraneCamera.h>
#include <TechnocraneCameraComponent.h>
#include <TechnocraneRigAnimInstance.h>

#define LOCTEXT_NAMESPACE "TechnocraneCamera"

///////////////////////////////////////////////////////////////////////////////////
// FTechnocraneRigImpl

class FTechnocraneRigImpl
{
public:

	//! a constructor
	FTechnocraneRigImpl()
	{}

	USkeletalMeshComponent* MeshComponent{ nullptr };
	FCraneData* CraneData{ nullptr };
	
	void SetPoseableMeshComponent(USkeletalMeshComponent* component)
	{
		MeshComponent = component;
	}

	void SetCranePresetData(FCraneData*	data)
	{
		CraneData = data;
	}

};

FTechnocraneRig::FTechnocraneRig()
{
	// allocate memory for a private data
	Impl = CreateImpl();
}
FTechnocraneRig::~FTechnocraneRig()
{
}

TUniquePtr<FTechnocraneRigImpl> FTechnocraneRig::CreateImpl()
{
	return TUniquePtr<FTechnocraneRigImpl>(new FTechnocraneRigImpl());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sets default values
ATechnocraneRig::ATechnocraneRig()
	: LastPreviewModel(ECranePreviewModelsEnum::ECranePreview_Count)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;

	// default control values
	TrackPosition = 0.0f;
	AmountOfTracks = 1;

	CameraPivotOffset = FVector(-70.0f, 0.0f, 0.0f);

	// create the root component
	TransformComponent = CreateDefaultSubobject<USceneComponent>(TEXT("TransformComponent"));
	RootComponent = TransformComponent;

#if WITH_EDITORONLY_DATA

	MeshComponent = nullptr;

	// create preview meshes
	if (!IsRunningDedicatedServer())
	{
		const FString DataPath = TEXT("/TechnocranePlugin/CranesData");
		static ConstructorHelpers::FObjectFinder<UDataTable> CraneDataAsset(*DataPath);
		if (CraneDataAsset.Succeeded())
		{
			CranesData = CraneDataAsset.Object;
			PreloadPreviewMeshes();
			PreloadTracksMesh();
		}
		else
		{
			CranesData = nullptr;
		}
	}
#endif
}

#if WITH_EDITORONLY_DATA

bool ATechnocraneRig::PreloadPreviewMeshes()
{
	if (!CranesData)
	{
		return false;
	}
		
	const TArray<FName> RowNames = CranesData->GetRowNames();

	for (const FName RawName : RowNames)
	{
		FString ModelPath("");

		if (FCraneData* Data = CranesData->FindRow<FCraneData>(RawName, "", false))
		{
			ModelPath = Data->CraneModelPath;
		}

		ConstructorHelpers::FObjectFinder<USkeletalMesh> CraneBaseMesh(*ModelPath);

		if (CraneBaseMesh.Succeeded())
		{
			PreviewMeshes.Add(CraneBaseMesh.Object);
		}
	}

	//

	MeshComponent = CreateOptionalDefaultSubobject<USkeletalMeshComponent>(TEXT("preview_mesh"));
	if (MeshComponent)
	{
		MeshComponent->bIsEditorOnly = true;
		MeshComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		MeshComponent->bHiddenInGame = false;
		MeshComponent->CastShadow = true;
		MeshComponent->SetupAttachment(TransformComponent);		// sibling of yawcontrol

		TechnocraneRig.Impl->SetPoseableMeshComponent(MeshComponent);
	}
	
	return (!PreviewMeshes.IsEmpty());
}

bool ATechnocraneRig::PreloadTracksMesh()
{
	const FString ModelPath("/TechnocranePlugin/tracks");

	ConstructorHelpers::FObjectFinder<UStaticMesh> CraneTracksBaseMesh(*ModelPath);

	if (CraneTracksBaseMesh.Succeeded())
	{
		CraneTracksMesh = CraneTracksBaseMesh.Object;
		CraneTracksMeshComponent = CreateOptionalDefaultSubobject<UStaticMeshComponent>(TEXT("tracks_mesh"));

		if (CraneTracksMeshComponent)
		{
			CraneTracksMeshComponent->bIsEditorOnly = true;
			CraneTracksMeshComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
			CraneTracksMeshComponent->bHiddenInGame = false;
			CraneTracksMeshComponent->CastShadow = true;
			CraneTracksMeshComponent->SetupAttachment(TransformComponent);		// sibling of yawcontrol

			CraneTracksMeshComponent->SetStaticMesh(CraneTracksMesh);
			return true;
		}
	}
	return false;
}

void ATechnocraneRig::UpdateTracksMesh()
{
	if (!CraneTracksMeshComponent)
		return;

	const bool bShowTracks = bLastSupportTracks && bShowTracksIfSupported;

	if (CraneTracksMeshComponent->IsVisible() != bShowTracks)
	{
		CraneTracksMeshComponent->SetVisibility(bShowTracks);
	}
}

void ATechnocraneRig::UpdatePreviewMeshes()
{
	if (PreviewMeshes.IsEmpty() || !MeshComponent)
	{
		return;
	}

	if (LastPreviewModel != CraneModel)
	{
		// attach another crane
		USkeletalMesh* PreviewMesh = PreviewMeshes[static_cast<int32>(CraneModel)];
			
		// crane preset data
		const FString PresetName(FString::FromInt(static_cast<int32>(CraneModel) + 1));
		if (FCraneData* Data = CranesData->FindRow<FCraneData>(FName(*PresetName), "", false))
		{
			TechnocraneRig.Impl->SetCranePresetData(Data);

			MeshComponent->SetSkinnedAssetAndUpdate(PreviewMesh);
			MeshComponent->SetAnimInstanceClass(UTechnocraneRigAnimInstance::StaticClass());

			TObjectPtr<UTechnocraneRigAnimInstance> AnimInstance = Cast<UTechnocraneRigAnimInstance>(MeshComponent->GetAnimInstance());

			if (AnimInstance)
			{
				AnimInstance->ConfigureAnimInstance(TargetComponent.OtherActor, *Data, CameraPivotOffset, bShowDebug);
				MeshComponent->SetUpdateAnimationInEditor(true);
				MeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
				MeshComponent->InitAnim(true /*bForceReinit*/);
			}

			LastPreviewModel = CraneModel;
			bLastSupportTracks = Data->TracksSupport > 0;
			LastTracksOffset = Data->ZOffsetOnTracks;

			if (CraneTracksMeshComponent)
			{
				CraneTracksMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, LastTracksOffset));
			}
		}
	}
}
#endif

void ATechnocraneRig::UpdateCraneComponents()
{
#if WITH_EDITORONLY_DATA
	UpdatePreviewMeshes();
	UpdateTracksMesh();
#endif
}

// Called when the game starts or when spawned
void ATechnocraneRig::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ATechnocraneRig::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// feed exposed API into underlying components
	UpdateCraneComponents();
}

#if WITH_EDITOR
void ATechnocraneRig::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (MeshComponent && PropertyChangedEvent.MemberProperty && TargetComponent.OtherActor.IsValid())
	{
		const FString PresetName(FString::FromInt(static_cast<int32>(CraneModel) + 1));
		if (FCraneData* Data = CranesData->FindRow<FCraneData>(FName(*PresetName), "", false))
		{
			TObjectPtr<UTechnocraneRigAnimInstance> AnimInstance = Cast<UTechnocraneRigAnimInstance>(MeshComponent->GetAnimInstance());

			if (AnimInstance)
			{
				AnimInstance->ConfigureAnimInstance(TargetComponent.OtherActor, *Data, CameraPivotOffset, bShowDebug);
			}
		}
	}

	UpdateCraneComponents();
}

void ATechnocraneRig::PostEditUndo()
{
	Super::PostEditUndo();

	UpdateCraneComponents();
}

void ATechnocraneRig::EditorApplyTranslation(const FVector& DeltaTranslation, bool bAltDown, bool bShiftDown, bool bCtrlDown)
{
	Super::EditorApplyTranslation(DeltaTranslation, bAltDown, bShiftDown, bCtrlDown);

	UpdateCraneComponents();
}

#endif // WITH_EDITOR

bool ATechnocraneRig::ShouldTickIfViewportsOnly() const
{
	return true;
}

#undef LOCTEXT_NAMESPACE