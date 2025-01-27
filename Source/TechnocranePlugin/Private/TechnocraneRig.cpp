// Copyright (c) 2020 Technocrane s.r.o. 
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

#include "LiveLinkComponentController.h"
#include "ILiveLinkClient.h"
#include "LiveLinkTypes.h"

#include "Roles/LiveLinkCameraRole.h"
#include "Roles/LiveLinkCameraTypes.h"
#include "LiveLinkTechnocraneTypes.h"

#define LOCTEXT_NAMESPACE "TechnocraneCamera"


///////////////////////////////////////////////////////////////////////////////////
// FTechnocraneRigImpl

class FTechnocraneRigImpl
{
public:

	//! a constructor
	FTechnocraneRigImpl()
	{}

	UPoseableMeshComponent* mComponent{ nullptr };
	FCraneData* CraneData{ nullptr };
	
	void SetPoseableMeshComponent(UPoseableMeshComponent* component)
	{
		mComponent = component;
	}

	void SetCranePresetData(FCraneData*	data)
	{
		CraneData = data;
	}

	FVector ComputeHeadTransform() const
	{
		const FName BoneName = GetCraneJointName(ECraneJoints::Head);
		return mComponent->GetBoneLocation(BoneName, EBoneSpaces::WorldSpace);
	}

	bool Reset()
	{
		mComponent->ResetBoneTransformByName(GetCraneJointName(ECraneJoints::Columns));
		mComponent->ResetBoneTransformByName(GetCraneJointName(ECraneJoints::Beams));
		return true;
	}

	// recompute transform of crane elements in order to fit a target camera position and orientation
	bool Compute(UWorld* World, const FVector& Target, const float TrackPosition, const FVector& RawRotation, const FQuat& NeckRotation)
	{
		if (!CraneData)
		{
			return false;
		}

		const FTransform& BaseTransform = mComponent->GetComponentTransform();

		const float ZOffset = CraneData->ZOffsetOnGround;		// make it appear in the right place
		FVector const NewLoc(0.0f, TrackPosition, ZOffset);
		
		const FTransform RelativeTM = BaseTransform.GetRelativeTransformReverse(FTransform(Target));
		FVector vTarget3 = Target; // relativeTM.GetLocation();

		const FName RootBoneName(GetCraneJointName(ECraneJoints::Base));
		const FName ColumnsBoneName(GetCraneJointName(ECraneJoints::Columns));
		const FName ColumnRotationBoneName(!CraneData->ColumnRotationBone.IsNone() ? CraneData->ColumnRotationBone : ColumnsBoneName);
		const FName BeamsBoneName(GetCraneJointName(ECraneJoints::Beams));
		const FName GravityBoneName(GetCraneJointName(ECraneJoints::Gravity));
		const FName Beam1BoneName(GetCraneJointName(ECraneJoints::Beam1));
		const FName Beam2BoneName(GetCraneJointName(ECraneJoints::Beam2));
		const FName Beam4BoneName(GetCraneJointName(ECraneJoints::Beam4));
		const FName NeckBoneName(GetCraneJointName(ECraneJoints::Neck));
		const FName HeadBoneName(GetCraneJointName(ECraneJoints::Head));

		mComponent->SetBoneLocationByName(RootBoneName, NewLoc, EBoneSpaces::ComponentSpace);

		FVector HeadPos;
		FVector ColumnPos, BeamsPos, ProjPos;
		
		FQuat DeltaQuat;
		FVector DeltaAxis(0.f);
		float DeltaAngle = 0.f;

		ColumnPos = mComponent->GetBoneLocation(ColumnsBoneName);
		BeamsPos = mComponent->GetBoneLocation(BeamsBoneName);

		{
			FRotator rot = mComponent->GetBoneRotationByName(ColumnRotationBoneName, EBoneSpaces::ComponentSpace);

			FVector DirInPlane = vTarget3 - BeamsPos;
			DirInPlane.Z = 0.0;
			DirInPlane.Normalize();

			double AngleRad = FMath::Atan2(FVector::DotProduct(FVector::CrossProduct(FVector::ForwardVector, DirInPlane), FVector::UpVector),
				FVector::DotProduct(DirInPlane, FVector::ForwardVector));
			double Angle = FMath::RadiansToDegrees(AngleRad);

			rot.Yaw = 90.0 + Angle;
			mComponent->SetBoneRotationByName(ColumnRotationBoneName, rot, EBoneSpaces::ComponentSpace);

			HeadPos = FVector::ForwardVector;
			FVector HeadDir = HeadPos.RotateAngleAxisRad(AngleRad, FVector::UpVector);
			HeadDir.Normalize();

			constexpr double DistCamHeadAndNeck{ 50.0 };
			FVector DirToCam = FVector(vTarget3.X, vTarget3.Y, vTarget3.Z + DistCamHeadAndNeck) - BeamsPos;
			DirToCam.Normalize();

			AngleRad = FMath::Acos(FVector::DotProduct(DirToCam, HeadDir));
			Angle = FMath::RadiansToDegrees( ((vTarget3.Z + DistCamHeadAndNeck) > BeamsPos.Z) ? AngleRad : -AngleRad);

			rot = mComponent->GetBoneRotationByName(BeamsBoneName, EBoneSpaces::ComponentSpace);
			rot.Roll = 90.0 + Angle;
			mComponent->SetBoneRotationByName(BeamsBoneName, rot, EBoneSpaces::ComponentSpace);
		}
		
		ColumnPos = mComponent->GetBoneLocation(ColumnsBoneName);
		BeamsPos = mComponent->GetBoneLocation(BeamsBoneName);
		HeadPos = ComputeHeadTransform();

		//
		// rotate gravity point
		FTransform TM, ParentTM;
		FVector angles;

		TM = mComponent->GetBoneTransformByName(BeamsBoneName, EBoneSpaces::ComponentSpace);
		ParentTM = mComponent->GetBoneTransformByName(mComponent->GetParentBone(BeamsBoneName), EBoneSpaces::ComponentSpace);

		TM.SetToRelativeTransform(ParentTM);
		angles = TM.GetRotation().Euler();

		TM = mComponent->GetBoneTransformByName(GravityBoneName, EBoneSpaces::ComponentSpace);
		ParentTM = mComponent->GetBoneTransformByName(mComponent->GetParentBone(GravityBoneName), EBoneSpaces::ComponentSpace);

		TM.SetToRelativeTransform(ParentTM);
		TM.SetRotation(FQuat::MakeFromEuler(-angles));

		TM = TM * ParentTM;
		mComponent->SetBoneTransformByName(GravityBoneName, TM, EBoneSpaces::ComponentSpace);

		//
		// beams length
		
		HeadPos = ComputeHeadTransform();

		DeltaQuat = FQuat::FindBetween(HeadPos - BeamsPos, ProjPos - BeamsPos);
		DeltaQuat.ToAxisAndAngle(DeltaAxis, DeltaAngle);

		if (DeltaAngle < 60.0f)
		{
			const float l1 = (HeadPos - BeamsPos).Size();
			const float l2 = (vTarget3 - BeamsPos).Size();
			const float l = 0.15f * (l1 - l2);

			if (abs(l) > 0.1f)
			{
				ParentTM = mComponent->GetBoneTransformByName(Beam1BoneName, EBoneSpaces::ComponentSpace);

				const int32 Beam2 = static_cast<int32>(ECraneJoints::Beam2);
				const int32 Beam5 = static_cast<int32>(ECraneJoints::Beam5);

				for (int32 i = Beam2; i <= Beam5; ++i)
				{
					const FName BoneName(GetCraneJointName(static_cast<ECraneJoints>(i)));
					
					if (INDEX_NONE == mComponent->GetBoneIndex(BoneName))
						continue;

					TM = mComponent->GetBoneTransformByName(BoneName, EBoneSpaces::ComponentSpace);
					TM.SetToRelativeTransform(ParentTM);
						
					FVector tr = TM.GetLocation();
					tr.Z += l;

					const float MaxLength = mComponent->GetRefPoseTransform(mComponent->GetBoneIndex(BoneName)).GetLocation().Z;

					if (tr.Z > -2.0f) tr.Z = -2.0f;
					else if (tr.Z < MaxLength) tr.Z = MaxLength;

					TM.SetLocation(tr);
					TM = TM * ParentTM;
						
					mComponent->SetBoneTransformByName(BoneName, TM, EBoneSpaces::ComponentSpace);

					ParentTM = TM;	
				}
			}
		}
		
		//
		// crane head and neck

		TM = mComponent->GetBoneTransformByName(NeckBoneName, EBoneSpaces::ComponentSpace);
		ParentTM = mComponent->GetBoneTransformByName(mComponent->GetParentBone(NeckBoneName), EBoneSpaces::ComponentSpace);

		TM.SetToRelativeTransform(ParentTM);
		
		TM = TM * ParentTM;
		TM.SetRotation(NeckRotation);

		mComponent->SetBoneTransformByName(NeckBoneName, TM, EBoneSpaces::ComponentSpace);

		//

		TM = mComponent->GetBoneTransformByName(HeadBoneName, EBoneSpaces::ComponentSpace);
		ParentTM = mComponent->GetBoneTransformByName(mComponent->GetParentBone(HeadBoneName), EBoneSpaces::ComponentSpace);

		TM.SetToRelativeTransform(ParentTM);
		TM.SetRotation(FQuat::MakeFromEuler(FVector(RawRotation.Y, 0.0f, 0.0f)));

		TM = TM * ParentTM;
		mComponent->SetBoneTransformByName(HeadBoneName, TM, EBoneSpaces::ComponentSpace);

		return true;
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
	PrimaryActorTick.TickInterval = 0;

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

	MeshComponent = CreateOptionalDefaultSubobject<UPoseableMeshComponent>(TEXT("preview_mesh"));
	if (MeshComponent)
	{
		MeshComponent->bIsEditorOnly = true;
		MeshComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		MeshComponent->bHiddenInGame = false;
		MeshComponent->CastShadow = true;
		MeshComponent->SetupAttachment(TransformComponent);		// sibling of yawcontrol

		TechnocraneRig.Impl->SetPoseableMeshComponent(MeshComponent);
	}
	
	return (PreviewMeshes.Num() > 0);
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

	bool bShowTracks = bLastSupportTracks && bShowTracksIfSupported;

	if (CraneTracksMeshComponent->IsVisible() != bShowTracks)
	{
		CraneTracksMeshComponent->SetVisibility(bShowTracks);
	}
}

void ATechnocraneRig::UpdatePreviewMeshes()
{
	
	if (PreviewMeshes.Num() > 0 && MeshComponent)
	{
		if (LastPreviewModel != CraneModel)
		{
			// attach another crane
			USkeletalMesh* PreviewMesh = PreviewMeshes[static_cast<int32>(CraneModel)];
			
			// crane preset data
			const FString PresetName(FString::FromInt(static_cast<int32>(CraneModel) + 1));
			FCraneData* Data = CranesData->FindRow<FCraneData>(FName(*PresetName), "", false);
			TechnocraneRig.Impl->SetCranePresetData(Data);

			MeshComponent->SetSkinnedAssetAndUpdate(PreviewMesh);
			
			LastPreviewModel = CraneModel;
			bLastSupportTracks = Data->TracksSupport > 0;
			LastTracksOffset = Data->ZOffsetOnTracks;

			if (CraneTracksMeshComponent)
			{
				CraneTracksMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, LastTracksOffset));
			}
		}

		// Perform kinematic updates

		FTransform Target = FTransform::Identity;
		Target.SetLocation(FVector(0.0f, 0.0f, 100.0f));
		FVector RawRotation = FVector::ZeroVector;
		FQuat NeckQ(FQuat::Identity);

		if (ACineCameraActor* CineCamera = Cast<ACineCameraActor>(TargetComponent.OtherActor))
		{
			// get transform and track position / raw rotation if live link is presented

			FTransform CameraTransform = CineCamera->GetTransform();
			
			if (UCineCameraComponent* comp = Cast<UCineCameraComponent>(CineCamera->GetCameraComponent()))
			{
				CameraTransform = comp->GetRelativeTransform() * CameraTransform;
			}
			if (UTechnocraneCameraComponent* Comp = Cast< UTechnocraneCameraComponent>(CineCamera->GetCameraComponent()))
			{
				TrackPosition = Comp->TrackPos;
			}

			// add some camera pivot offset
			FTransform NewPivotOffset;
			NewPivotOffset.SetLocation(CameraPivotOffset);

			Target = NewPivotOffset * CameraTransform;
			RawRotation = CineCamera->GetActorRotation().Euler();
			NeckQ = FQuat::MakeFromEuler(FVector(90.0, 0.0, 180.0 + RawRotation.X));

			if (!CameraPivotOffset.IsNearlyZero(0.0001))
			{
				FVector DirToCam = (CameraTransform.GetLocation() - Target.GetLocation());
				DirToCam.Normalize();
				FVector DirInPlane = DirToCam;
				DirInPlane.Z = 0.0;
				DirInPlane.Normalize();
				
				double AngleRad = FMath::Atan2(FVector::DotProduct(FVector::CrossProduct(FVector::ForwardVector, DirInPlane), FVector::UpVector),
					FVector::DotProduct(DirInPlane, FVector::ForwardVector));

				const double Angle = FMath::RadiansToDegrees( AngleRad );

				NeckQ = FQuat::MakeFromEuler(FVector(90.0, 0.0, 90.0 + Angle));
			}

			// track position / raw rotation

			ULiveLinkComponentController* LiveLinkComponent = Cast<ULiveLinkComponentController>(CineCamera->GetComponentByClass(ULiveLinkComponentController::StaticClass()));
			if (LiveLinkComponent && LiveLinkComponent->SubjectRepresentation.Role)
			{
				//if Subjects role direct controller is us, set the component to control to what we had
				
				if (ULiveLinkCameraRole* LiveLinkRole = Cast<ULiveLinkCameraRole>(LiveLinkComponent->SubjectRepresentation.Role->GetDefaultObject()))
				{
					//Create the struct holder and make it point to the output data
					
					IModularFeatures& ModularFeatures = IModularFeatures::Get();
					if (ModularFeatures.IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName))
					{
						ILiveLinkClient& LiveLinkClient = ModularFeatures.GetModularFeature<ILiveLinkClient>(ILiveLinkClient::ModularFeatureName);
						
						FLiveLinkSubjectFrameData CurrentFrameData;
						bool bSuccess = LiveLinkClient.EvaluateFrame_AnyThread(LiveLinkComponent->SubjectRepresentation.Subject, LiveLinkComponent->SubjectRepresentation.Role, CurrentFrameData);
							
						if (bSuccess)
						{
							FLiveLinkCameraFrameData* FrameData = CurrentFrameData.FrameData.Cast<FLiveLinkCameraFrameData>();

							if (FrameData->PropertyValues.Num() > 8)
							{
								TrackPosition = FrameData->PropertyValues[static_cast<int32>(EPacketProperties::TrackPosition)];
								RawRotation = FVector(
									FrameData->PropertyValues[static_cast<int32>(EPacketProperties::Pan)],
									FrameData->PropertyValues[static_cast<int32>(EPacketProperties::Tilt)],
									FrameData->PropertyValues[static_cast<int32>(EPacketProperties::Roll)]
								);

								NeckQ = FQuat::MakeFromEuler(FVector(90.0f, 0.0f, 180.0f + RawRotation.X));
							}
						}
					}
				}
			}
			
		}
		
		TechnocraneRig.Impl->Compute(GetWorld(), Target.GetLocation(), TrackPosition, RawRotation, NeckQ);
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

	if (PropertyChangedEvent.MemberProperty && PropertyChangedEvent.MemberProperty->GetFName() == "TargetComponent")
	{
		TechnocraneRig.Impl->Reset();
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
