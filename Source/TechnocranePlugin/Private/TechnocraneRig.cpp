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

	FVector ComputeHeadWorldTransform() const
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

	// Beam structure to hold individual beam properties
	struct FBeamData
	{
		float CurrentLength;
		float MinLength;        // Absolute minimum length the beam can have
		float MaxLength;        // Absolute maximum length the beam can have
		float Adjustment;       // Output: how much to adjust this beam (+/- value)
		FName BeamBoneName;

		FBeamData(float Current, float Min, float Max, const FName& BoneName)
			: CurrentLength(Current), MinLength(Min), MaxLength(Max), Adjustment(0.0f), BeamBoneName(BoneName) {}

		float GetAdjustedLength() const { return CurrentLength + Adjustment; }

		// How much we can extend this beam (positive value)
		float GetMaxPossibleExtension() const { return MaxLength - CurrentLength; }

		// How much we can shrink this beam (negative value)  
		float GetMaxPossibleShrinkage() const { return MinLength - CurrentLength; }

		// Total adjustment range available for this beam
		float GetTotalAdjustmentRange() const { return MaxLength - MinLength; }
	};

	// Alternative algorithm: Equal distribution with overflow handling
	void CalculateBeamAdjustmentsEqual(TArray<FBeamData, TInlineAllocator<4>>& Beams, float CurrentTotalLength, float TargetLength, float BaseBeamLength)
	{
		if (Beams.IsEmpty())
			return;

		const float RequiredAdjustment = TargetLength - BaseBeamLength;
		const float AdjustmentPerBeam = RequiredAdjustment / Beams.Num();
		float RemainingAdjustment = RequiredAdjustment;

		// Reset all adjustments
		for (FBeamData& Beam : Beams)
		{
			Beam.Adjustment = 0.0f;
		}

		if (FMath::IsNearlyZero(RequiredAdjustment))
		{
			return; // Already at target, no adjustments needed
		}

		// First pass: try to distribute equally
		for (FBeamData& Beam : Beams)
		{
			const float ClampedAdjustment = FMath::Clamp(AdjustmentPerBeam, Beam.MinLength, Beam.MaxLength);

			Beam.Adjustment = ClampedAdjustment;
			RemainingAdjustment -= ClampedAdjustment;
		}

		constexpr float Thres{ 0.001f };

		// Second pass: distribute remaining adjustment to beams that can still accommodate it
		while (!FMath::IsNearlyZero(RemainingAdjustment) && FMath::Abs(RemainingAdjustment) > Thres)
		{
			int32 AvailableBeams = 0;

			// Count beams that can still be adjusted
			for (int32 i = 0; i < Beams.Num(); i++)
			{
				FBeamData& Beam = Beams[i];
				if (RemainingAdjustment > 0.0f && Beam.GetAdjustedLength() < Beam.MaxLength - Thres)
				{
					AvailableBeams++;
				}
				else if (RemainingAdjustment < 0.0f && Beam.GetAdjustedLength() > Beam.MinLength + Thres)
				{
					AvailableBeams++;
				}
			}

			if (AvailableBeams == 0) break;

			float AdjustmentPerAvailableBeam = RemainingAdjustment / AvailableBeams;
			float OldRemainingAdjustment = RemainingAdjustment;

			for (int32 i = 0; i < Beams.Num(); i++)
			{
				FBeamData& Beam = Beams[i];
				bool CanAdjust = false;

				if (RemainingAdjustment > 0.0f 
					&& (Beam.GetAdjustedLength() < Beam.MaxLength - Thres || Beam.GetAdjustedLength() > Beam.MinLength + Thres))
				{
					CanAdjust = true;
				}
				
				if (CanAdjust)
				{
					const float AdditionalAdjustment = FMath::Clamp(AdjustmentPerAvailableBeam, Beam.MinLength, Beam.MaxLength);

					Beam.Adjustment += AdditionalAdjustment;
					RemainingAdjustment -= AdditionalAdjustment;
				}
			}

			// Prevent infinite loop
			if (FMath::Abs(RemainingAdjustment - OldRemainingAdjustment) < 0.001f)
			{
				break;
			}
		}
	}

	// recompute transform of crane elements in order to fit a target camera position and orientation
	bool Compute(UWorld* World, FCraneSimulationData& OutCraneData, const FVector& Target, const float TrackPosition, const FVector& RawRotation, const FQuat& NeckRotation)
	{
		if (!CraneData)
		{
			return false;
		}

		const float ZOffset = CraneData->ZOffsetOnGround;		// make it appear in the right place
		FVector const NewLoc(0.0f, TrackPosition, ZOffset);
		
		OutCraneData.GroundHeight = Target.Z;

		FVector vTarget3 = Target;

		const FName RootBoneName(GetCraneJointName(ECraneJoints::Base));
		const FName ColumnsBoneName(GetCraneJointName(ECraneJoints::Columns));
		const FName ColumnRotationBoneName(!CraneData->ColumnRotationBone.IsNone() ? CraneData->ColumnRotationBone : ColumnsBoneName);
		const FName BeamsBoneName(GetCraneJointName(ECraneJoints::Beams));
		const FName GravityBoneName(GetCraneJointName(ECraneJoints::Gravity));
		const FName Beam1BoneName(GetCraneJointName(ECraneJoints::Beam1));
		const FName NeckBoneName(GetCraneJointName(ECraneJoints::Neck));
		const FName HeadBoneName(GetCraneJointName(ECraneJoints::Head));

		mComponent->SetBoneLocationByName(RootBoneName, NewLoc, EBoneSpaces::ComponentSpace);

		FVector HeadPos;
		FVector DeltaAxis(0.f);
		float DeltaAngle = 0.f;

		FVector ColumnPos = mComponent->GetBoneLocation(ColumnsBoneName);
		FVector BeamsPos = mComponent->GetBoneLocation(BeamsBoneName);

		{
			//
			// rotate around UP

			FRotator ColumnRot = mComponent->GetBoneRotationByName(ColumnRotationBoneName, EBoneSpaces::ComponentSpace);
			const FVector DirInPlane = (vTarget3 - BeamsPos).GetSafeNormal2D();

			double AngleRad = FMath::Atan2(FVector::DotProduct(FVector::CrossProduct(FVector::ForwardVector, DirInPlane), FVector::UpVector),
				FVector::DotProduct(DirInPlane, FVector::ForwardVector));
			double Angle = FMath::RadiansToDegrees(AngleRad);

			ColumnRot.Yaw = 90.0 + Angle;
			mComponent->SetBoneRotationByName(ColumnRotationBoneName, ColumnRot, EBoneSpaces::ComponentSpace);
		
			const FVector HeadDir = FVector::ForwardVector.RotateAngleAxisRad(AngleRad, FVector::UpVector).GetSafeNormal();
		
			//
			// dist from gravity pivot up to camera pivot

			// Y is a local down direction in the skeleton
			const float GravityPivotZ = mComponent->GetRefPosePosition(mComponent->GetBoneIndex(GravityBoneName)).Y;
			const float NeckPivotZ = mComponent->GetRefPosePosition(mComponent->GetBoneIndex(NeckBoneName)).Length();
			const float CameraPivotZ = mComponent->GetRefPosePosition(mComponent->GetBoneIndex(HeadBoneName)).Length();

			const float DistCamHeadAndNeck = FMath::Abs(GravityPivotZ) + FMath::Abs(CameraPivotZ) + FMath::Abs(NeckPivotZ);

			//
			// rotate beams up/down

			FVector DirToCam = FVector(vTarget3.X, vTarget3.Y, vTarget3.Z + DistCamHeadAndNeck) - BeamsPos;
			DirToCam.Normalize();

			AngleRad = FMath::Acos(FVector::DotProduct(DirToCam, HeadDir));
			Angle = FMath::RadiansToDegrees( ((vTarget3.Z + DistCamHeadAndNeck) > BeamsPos.Z) ? AngleRad : -AngleRad);

			Angle = FMath::Clamp(Angle, -CraneData->TiltMin, CraneData->TiltMax);

			FRotator BeamsRot = mComponent->GetBoneRotationByName(BeamsBoneName, EBoneSpaces::ComponentSpace);
			BeamsRot.Roll = 90.0 + Angle;
			mComponent->SetBoneRotationByName(BeamsBoneName, BeamsRot, EBoneSpaces::ComponentSpace);

			OutCraneData.TiltAngle = Angle;
		}
		
		ColumnPos = mComponent->GetBoneLocation(ColumnsBoneName);
		BeamsPos = mComponent->GetBoneLocation(BeamsBoneName);
		HeadPos = ComputeHeadWorldTransform();

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
		
		HeadPos = ComputeHeadWorldTransform();

		// we assume crane crane beams can't be placed almost vertically and crane preset has a defined tilt min/max angles setup
		const float CurrentLength = FVector::Dist(HeadPos, BeamsPos);
		const float TargetLength = FVector::Dist(vTarget3, BeamsPos);
		OutCraneData.ExtensionLength = TargetLength;

		constexpr float Thres{ 0.1f };
		if (FMath::Abs(CurrentLength - TargetLength) > Thres)
		{
			TArray<FBeamData, TInlineAllocator<4>> BeamData;

			const int32 Beam1 = static_cast<int32>(ECraneJoints::Beam1);
			const int32 Beam2 = static_cast<int32>(ECraneJoints::Beam2);
			const int32 Beam5 = static_cast<int32>(ECraneJoints::Beam5);

			for (int32 i = Beam2; i <= Beam5; ++i)
			{
				const FName BoneName(GetCraneJointName(static_cast<ECraneJoints>(i)));
				const int32 BoneIndex = mComponent->GetBoneIndex(BoneName);

				if (INDEX_NONE == BoneIndex)
					continue;

				const float BeamLength = mComponent->GetBoneLocation(BoneName, EBoneSpaces::ComponentSpace).Z;
				const float MaxLength = -mComponent->GetRefPosePosition(BoneIndex).Z;
				constexpr float MinLength{ -2.0f };

				FBeamData Data(BeamLength, MinLength, MaxLength, BoneName);
				BeamData.Add(MoveTemp(Data));
			}

			const int32 Beam1BoneIndex = mComponent->GetBoneIndex(GetCraneJointName(ECraneJoints::Beam1));
			const float Beam1Length = -mComponent->GetRefPosePosition(Beam1BoneIndex).Z;
			
			CalculateBeamAdjustmentsEqual(BeamData, CurrentLength, TargetLength, Beam1Length);

			for (const FBeamData Data : BeamData)
			{
				const FName& BoneName = Data.BeamBoneName;
				
				ParentTM = mComponent->GetBoneTransformByName(mComponent->GetParentBone(BoneName), EBoneSpaces::ComponentSpace);

				TM = mComponent->GetBoneTransformByName(BoneName, EBoneSpaces::ComponentSpace);
				TM.SetToRelativeTransform(ParentTM);

				FVector tr = TM.GetLocation();
				tr.Z = -Data.Adjustment;
				
				TM.SetLocation(tr);
				TM = TM * ParentTM;

				mComponent->SetBoneTransformByName(BoneName, TM, EBoneSpaces::ComponentSpace);
			}
		}
		
		//
		// crane head and neck

		mComponent->SetBoneRotationByName(NeckBoneName, NeckRotation.Rotator(), EBoneSpaces::ComponentSpace);		
		mComponent->ResetBoneTransformByName(HeadBoneName);
		mComponent->BoneSpaceTransforms[mComponent->GetBoneIndex(HeadBoneName)].SetRotation(FQuat::MakeFromEuler(FVector(RawRotation.Y, 0.0f, 0.0f)));
		
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
		
		TechnocraneRig.Impl->Compute(GetWorld(), SimulationData, Target.GetLocation(), TrackPosition, RawRotation, NeckQ);
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