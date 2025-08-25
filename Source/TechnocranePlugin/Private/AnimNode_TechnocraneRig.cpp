// Copyright (c) 2025 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// AnimNode_TechnocraneRig.cpp
// Sergei <Neill3d> Solokhin

#include "AnimNode_TechnocraneRig.h"

#include "Animation/AnimCurveUtils.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"

#include <Runtime/CinematicCamera/Public/CineCameraActor.h>
#include <Runtime/CinematicCamera/Public/CineCameraComponent.h>
#include <TechnocraneCameraComponent.h>
#include "TechnocraneShared.h"

#include "LiveLinkComponentController.h"
#include "ILiveLinkClient.h"
#include "LiveLinkTypes.h"

#include "Roles/LiveLinkCameraRole.h"
#include "Roles/LiveLinkCameraTypes.h"
#include "LiveLinkTechnocraneTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AnimNode_TechnocraneRig)

UE_DISABLE_OPTIMIZATION

namespace NTechnocraneRigInternal
{
	// Beam structure to hold individual beam properties
	struct FBeamData
	{
		float CurrentLength;
		float MinLength;        // Absolute minimum length the beam can have
		float MaxLength;        // Absolute maximum length the beam can have
		float Adjustment;       // Output: how much to adjust this beam (+/- value)
		FCompactPoseBoneIndex BeamBone;
		FCompactPoseBoneIndex ParentBone;
		FName BeamBoneName;

		FBeamData(float Current, float Min, float Max, FCompactPoseBoneIndex InBone, FCompactPoseBoneIndex InParent, const FName& BoneName)
			: CurrentLength(Current), MinLength(Min), MaxLength(Max), Adjustment(0.0f), BeamBone(InBone), ParentBone(InParent), BeamBoneName(BoneName) {}

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

	float GetSignedAngle(const FVector& RefNormalX, FVector& RefNormalY, const FVector& RefTangent)
	{
		const FVector CrossProduct = FVector::CrossProduct(RefNormalX, RefNormalY);
		const float PositiveAngle = atan2(CrossProduct.Length(), FVector::DotProduct(RefNormalX, RefNormalY));
		return (FVector::DotProduct(RefTangent, CrossProduct) < 0.0) ? -PositiveAngle : PositiveAngle;
	}
};


void FAnimNode_TechnocraneRig::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Initialize_AnyThread)

	FAnimNode_Base::Initialize_AnyThread(Context);

	GetEvaluateGraphExposedInputs().Execute(Context);
}

void FAnimNode_TechnocraneRig::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(CacheBones_AnyThread)

	FAnimNode_Base::CacheBones_AnyThread(Context);

	const FBoneContainer& RequiredBones = Context.AnimInstanceProxy->GetRequiredBones();
	if (!RequiredBones.IsValid())
	{
		return;
	}

	const USkeletalMesh* SkeletalMesh = RequiredBones.GetSkeletalMeshAsset();
	const FReferenceSkeleton& MeshRefSkeleton = SkeletalMesh->GetRefSkeleton();

	ColumnRotationBone = ECraneJoints::Columns;

	const int32 CraneJointsCount = static_cast<int32>(ECraneJoints::JointCount);
	for (int32 i = 0; i < CraneJointsCount; ++i)
	{
		const ECraneJoints JointId = static_cast<ECraneJoints>(i);
		const FName JointName(GetCraneJointName(JointId));

		if (CraneData.ColumnRotationBone == JointName)
		{
			ColumnRotationBone = JointId;
		}

		int32 JointRefIndex = MeshRefSkeleton.FindBoneIndex(JointName);
		if (JointRefIndex < 0 && JointId == ECraneJoints::Base)
		{
			JointRefIndex = 0; // make a default root bone as base in case the given joint name is not found
		}
		const int32 ParentRefIndex = (JointRefIndex != INDEX_NONE) ? MeshRefSkeleton.GetParentIndex(JointRefIndex) : INDEX_NONE;

		const FMeshPoseBoneIndex BoneIndex = FMeshPoseBoneIndex(JointRefIndex);
		const FMeshPoseBoneIndex ParentBoneIndex = FMeshPoseBoneIndex(ParentRefIndex);

		CraneJointToCompactBoneIndex.Emplace(JointId, MakeTuple(RequiredBones.MakeCompactPoseIndex(BoneIndex), RequiredBones.MakeCompactPoseIndex(ParentBoneIndex)));
	}

	const FName GravityBoneName(GetCraneJointName(ECraneJoints::Gravity));
	const FName NeckBoneName(GetCraneJointName(ECraneJoints::Neck));
	const FName HeadBoneName(GetCraneJointName(ECraneJoints::Head));

	const auto& Transforms = MeshRefSkeleton.GetRefBonePose();
	FVector GravityLocal = Transforms[MeshRefSkeleton.FindBoneIndex(GravityBoneName)].GetLocation();
	const float GravityPivotZ = GravityLocal.Y;
	GravityOffsetLen = FMath::Sqrt(FMath::Square(GravityLocal.X) + FMath::Square(GravityLocal.Z)); // GravityLocal.Size2D(); // length in a horizontal plane
	const float NeckPivotZ = Transforms[MeshRefSkeleton.FindBoneIndex(NeckBoneName)].GetLocation().Length();
	const float CameraPivotZ = Transforms[MeshRefSkeleton.FindBoneIndex(HeadBoneName)].GetLocation().Length();

	DistCamHeadAndNeck = FMath::Abs(GravityPivotZ) + FMath::Abs(CameraPivotZ) + FMath::Abs(NeckPivotZ);
}

void FAnimNode_TechnocraneRig::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Update_AnyThread)

	FAnimNode_Base::Update_AnyThread(Context);

	// this introduces a frame of latency in setting the pin-driven source component,
    // but we cannot do the work to extract transforms on a worker thread as it is not thread safe.
    GetEvaluateGraphExposedInputs().Execute(Context);
}



void FAnimNode_TechnocraneRig::Evaluate_AnyThread(FPoseContext& Output)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Evaluate_AnyThread)
	SCOPE_CYCLE_COUNTER(STAT_TechnocraneRig);
	
	if (!TargetCameraActor.IsValid() || CraneJointToCompactBoneIndex.IsEmpty())
	{
		Output.ResetToRefPose();
		return;
	}

	const FCompactPoseBoneIndex RootBone = CraneJointToCompactBoneIndex.FindChecked(ECraneJoints::Base).Key;
	const FCompactPoseBoneIndex ColumnBone = CraneJointToCompactBoneIndex.FindChecked(ColumnRotationBone).Key;
	const FCompactPoseBoneIndex BeamsBone = CraneJointToCompactBoneIndex.FindChecked(ECraneJoints::Beams).Key;
	const FCompactPoseBoneIndex GravityBone = CraneJointToCompactBoneIndex.FindChecked(ECraneJoints::Gravity).Key;
	const FCompactPoseBoneIndex NeckBone = CraneJointToCompactBoneIndex.FindChecked(ECraneJoints::Neck).Key;
	const FCompactPoseBoneIndex HeadBone = CraneJointToCompactBoneIndex.FindChecked(ECraneJoints::Head).Key;

	// convert pose to local space and apply to output
	FCSPose<FCompactPose> ComponentPose;
	ComponentPose.InitPose(Output.Pose);
	
	// TODO: deliver back to rig actor ?!
	FCraneSimulationData OutCraneData;

	const float ZOffset = CraneData.ZOffsetOnGround;		// make it appear in the right place
	const FVector NewLoc(0.0f, TrackPosition, ZOffset);

	FVector vTarget3 = Target.GetLocation();
	OutCraneData.GroundHeight = vTarget3.Z;

	FTransform RootTM = ComponentPose.GetComponentSpaceTransform(RootBone);
	RootTM.SetLocation(NewLoc);
	ComponentPose.SetComponentSpaceTransform(RootBone, RootTM);

	TArray<FCompactPoseBoneIndex> ModifiedBones;
	ModifiedBones.Add(RootBone);

	FTransform ColumnTransform;

	{
		//
		// rotate around UP
		ColumnTransform = ComponentPose.GetComponentSpaceTransform(ColumnBone);
		FVector ColumnPos = ColumnTransform.GetLocation();

		FRotator ColumnRot = ColumnTransform.Rotator();
		const FVector DirInPlane = vTarget3.GetSafeNormal2D();

		const double AngleRad = FMath::Atan2(FVector::DotProduct(FVector::CrossProduct(FVector::ForwardVector, DirInPlane), FVector::UpVector),
			FVector::DotProduct(DirInPlane, FVector::ForwardVector));
		const double Angle = FMath::RadiansToDegrees(AngleRad);


		ColumnRot.Yaw = 90.0 + Angle;
		ColumnTransform.SetRotation(ColumnRot.Quaternion());
		ComponentPose.SetComponentSpaceTransform(ColumnBone, ColumnTransform);
		ModifiedBones.Add(ColumnBone);
	}

	{
		//
		// rotate beams up/down
		
		FTransform BeamsTransform = ComponentPose.GetComponentSpaceTransform(BeamsBone);
		FVector BeamsPos = BeamsTransform.GetLocation();

		FVector DirToCam = FVector(vTarget3.X, vTarget3.Y, vTarget3.Z + DistCamHeadAndNeck) - BeamsPos;
		DirToCam.Normalize();

		FVector vTarget3Norm = vTarget3.GetSafeNormal2D();
		float Angle = NTechnocraneRigInternal::GetSignedAngle(DirToCam, vTarget3Norm, ColumnTransform.GetRotation().GetForwardVector());
		Angle = FMath::RadiansToDegrees(Angle);
		Angle = FMath::Clamp(Angle, -CraneData.TiltMin, CraneData.TiltMax);

		// 
		FRotator BeamsRot = BeamsTransform.Rotator();
		BeamsRot.Roll = 90.0 + Angle;
		BeamsTransform.SetRotation(BeamsRot.Quaternion());
		ComponentPose.SetComponentSpaceTransform(BeamsBone, BeamsTransform);
		ModifiedBones.Add(BeamsBone);

		OutCraneData.TiltAngle = Angle;
	}

	//
	// beams length

	FVector HeadPos = ComponentPose.GetComponentSpaceTransform(HeadBone).GetLocation();
	FVector BeamsPos = ComponentPose.GetComponentSpaceTransform(BeamsBone).GetLocation();
	
	// we assume crane crane beams can't be placed almost vertically and crane preset has a defined tilt min/max angles setup
	const float CurrentLength = FVector::Dist(HeadPos, BeamsPos);
	const FVector ProjOnBeams = vTarget3 + FVector(0.0f, 0.0f, DistCamHeadAndNeck);
	//DrawDebugSphere(Output.AnimInstanceProxy->GetSkeleton()->GetWorld(), ProjOnBeams, 5.0f, 12, FColor::Red, false, 0.033f, SDPG_Foreground);
	const float TargetLength = FVector::Dist(ProjOnBeams, BeamsPos);
	OutCraneData.ExtensionLength = TargetLength;

	using namespace NTechnocraneRigInternal;
	const FReferenceSkeleton& RefSkeleton = Output.AnimInstanceProxy->GetSkeleton()->GetReferenceSkeleton();

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
			const int32 BoneIndex = RefSkeleton.FindBoneIndex(BoneName);
			
			if (INDEX_NONE == BoneIndex)
				continue;

			auto& BoneInfo = CraneJointToCompactBoneIndex.FindChecked(static_cast<ECraneJoints>(i));
			const FCompactPoseBoneIndex BeamBone = BoneInfo.Key;

			const float BeamLength = ComponentPose.GetComponentSpaceTransform(BeamBone).GetLocation().Length(); // Z;
			const float MaxLength = RefSkeleton.GetRefBonePose()[BoneIndex].GetLocation().Length(); // Z;
			constexpr float MinLength{ -2.0f };

			FBeamData Data(BeamLength, MinLength, MaxLength, BeamBone, BoneInfo.Value, BoneName);
			BeamData.Add(MoveTemp(Data));
		}

		const int32 Beam1BoneIndex = RefSkeleton.FindBoneIndex(GetCraneJointName(ECraneJoints::Beam1));
		const FVector Beam1Local = RefSkeleton.GetRefBonePose()[Beam1BoneIndex].GetLocation();
		const float Beam1Length = Beam1Local.Length();
		float TiltFactor = FMath::Sin(FMath::DegreesToRadians(90.0 - OutCraneData.TiltAngle));

		CalculateBeamAdjustmentsEqual(BeamData, CurrentLength, TargetLength - GravityOffsetLen, Beam1Length);

		for (const FBeamData& Data : BeamData)
		{
			const FName& BoneName = Data.BeamBoneName;

			FTransform ParentTM = ComponentPose.GetComponentSpaceTransform(Data.ParentBone);

			FTransform TM = ComponentPose.GetComponentSpaceTransform(Data.BeamBone);
			TM.SetToRelativeTransform(ParentTM);

			FVector tr = TM.GetLocation();
			tr.Z = -Data.Adjustment;

			TM.SetLocation(tr);
			TM = TM * ParentTM;

			ComponentPose.SetComponentSpaceTransform(Data.BeamBone, TM);
			ModifiedBones.Add(Data.BeamBone);
		}
	}
	
	{
		//
		// rotate gravity point

		FTransform TM, ParentTM;
		FVector angles;

		auto& BeamsBoneInfo = CraneJointToCompactBoneIndex.FindChecked(ECraneJoints::Beams);
		auto& GravityBoneInfo = CraneJointToCompactBoneIndex.FindChecked(ECraneJoints::Gravity);

		// convert to local space
		FCompactPose CompactPose(Output.Pose);
		FCSPose<FCompactPose>::ConvertComponentPosesToLocalPosesSafe(ComponentPose, CompactPose);
		
		// reset to ref pose before setting the pose to ensure if we don't have any missing bones
		Output.ResetToRefPose();

		for (const FCompactPoseBoneIndex& ModifiedBone : ModifiedBones)
		{
			Output.Pose[ModifiedBone] = CompactPose[ModifiedBone];
		}
		
		ComponentPose.InitPose(Output.Pose);

		TM = ComponentPose.GetComponentSpaceTransform(BeamsBoneInfo.Key);
		ParentTM = ComponentPose.GetComponentSpaceTransform(BeamsBoneInfo.Value);

		TM.SetToRelativeTransform(ParentTM);
		angles = TM.GetRotation().Euler();

		TM = ComponentPose.GetComponentSpaceTransform(GravityBoneInfo.Key);
		ParentTM = ComponentPose.GetComponentSpaceTransform(GravityBoneInfo.Value);

		TM.SetToRelativeTransform(ParentTM);
		TM.SetRotation(FQuat::MakeFromEuler(-angles));

		TM = TM * ParentTM;
		ComponentPose.SetComponentSpaceTransform(GravityBoneInfo.Key, TM);
	}
	
	//
	// crane head and neck

	FTransform HeadTM = ComponentPose.GetComponentSpaceTransform(HeadBone);
	HeadTM.SetRotation(NeckQ * FQuat::MakeFromEuler(FVector(RawRotation.Y, 0.0f, 0.0f)));
	ComponentPose.SetComponentSpaceTransform(HeadBone, HeadTM);

	FTransform NeckTM = ComponentPose.GetComponentSpaceTransform(NeckBone);
	NeckTM.SetRotation(NeckQ);
	ComponentPose.SetComponentSpaceTransform(NeckBone, NeckTM);

	// convert to local space
	FCSPose<FCompactPose>::ConvertComponentPosesToLocalPoses(ComponentPose, Output.Pose);
}

void FAnimNode_TechnocraneRig::PreUpdate(const UAnimInstance* InAnimInstance)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(PreUpdate)

	if (!TargetCameraActor.IsValid())
	{
		return;
	}

	// get transform and track position / raw rotation if live link is presented

	FTransform CameraTransform = TargetCameraActor->GetTransform();

	if (UCineCameraComponent* CameraComp = Cast<UCineCameraComponent>(TargetCameraActor->GetCameraComponent()))
	{
		CameraTransform = CameraComp->GetRelativeTransform() * CameraTransform;
	}
	if (UTechnocraneCameraComponent* CameraComp = Cast<UTechnocraneCameraComponent>(TargetCameraActor->GetCameraComponent()))
	{
		TrackPosition = CameraComp->TrackPos;
	}

	// add some camera pivot offset

	FVector AdjOffset = FVector::ZeroVector;
	AdjOffset = TargetCameraActor->GetActorForwardVector() * CameraPivotOffset.X;
	AdjOffset += -TargetCameraActor->GetActorRightVector() * CameraPivotOffset.Y;
	AdjOffset += TargetCameraActor->GetActorUpVector() * CameraPivotOffset.Z;

	Target = CameraTransform;
	Target.AddToTranslation(AdjOffset);

	if (bShowDebug)
	{
		DrawDebugSphere(InAnimInstance->GetWorld(), Target.GetLocation(), 5.0f, 12, FColor::White, false, 0.033f, SDPG_Foreground);
	}
	
	RawRotation = TargetCameraActor->GetActorRotation().Euler();
	NeckQ = FQuat::MakeFromEuler(FVector(90.0, 0.0, 180.0 + RawRotation.X));

	if (!CameraPivotOffset.IsNearlyZero(0.0001))
	{
		FVector DirToCam = (CameraTransform.GetLocation() - Target.GetLocation()).GetSafeNormal();
		FVector DirInPlane = DirToCam.GetSafeNormal2D();

		double AngleRad = FMath::Atan2(FVector::DotProduct(FVector::CrossProduct(FVector::ForwardVector, DirInPlane), FVector::UpVector),
			FVector::DotProduct(DirInPlane, FVector::ForwardVector));

		const double Angle = FMath::RadiansToDegrees(AngleRad);

		NeckQ = FQuat::MakeFromEuler(FVector(90.0, 0.0, 90.0 + Angle));
	}

	const FTransform OwnerTM = InAnimInstance->GetOwningActor()->GetTransform();
	Target = Target.GetRelativeTransform(OwnerTM);

	// TODO: live link stuff, track position / raw rotation
	
	ULiveLinkComponentController* LiveLinkComponent = Cast<ULiveLinkComponentController>(TargetCameraActor->GetComponentByClass(ULiveLinkComponentController::StaticClass()));
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

bool FAnimNode_TechnocraneRig::Serialize(FArchive& Ar)
{
	return false;
}

void FAnimNode_TechnocraneRig::PostSerialize(const FArchive& Ar)
{
}

UE_ENABLE_OPTIMIZATION