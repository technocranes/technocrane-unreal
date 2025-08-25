// Copyright (c) 2025 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// AnimNode_TechnocraneRig.h
// Sergei <Neill3d> Solokhin

#pragma once

#include "CoreMinimal.h"

#include "Animation/AnimNodeBase.h"
#include "TechnocraneData.h"
#include "TechnocraneShared.h"
#include "AnimNode_TechnocraneRig.generated.h"

DECLARE_CYCLE_STAT(TEXT("Technocrane Rig"), STAT_TechnocraneRig, STATGROUP_Anim);

// forward
class ACineCameraActor;

USTRUCT(BlueprintInternalUseOnly)
struct FAnimNode_TechnocraneRig : public FAnimNode_Base
{
	GENERATED_BODY()
	
	// Input pose to be modified by the retargeter when using "Source Pose Pin" mode as the Input Pose Mode.
	UPROPERTY(EditAnywhere, Category = Links)
	FPoseLink Source;
	
	// The actor which are going to follow with a crane simulation
	UPROPERTY(BlueprintReadWrite, transient, Category = Settings, meta=(PinShownByDefault))
	TWeakObjectPtr<ACineCameraActor> TargetCameraActor = nullptr;
	
	UPROPERTY(BlueprintReadWrite, transient, Category = Settings, meta = (PinShownByDefault))
	FVector CameraPivotOffset = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, transient, Category = Settings, meta = (PinShownByDefault))
	FCraneData CraneData;

	UPROPERTY(BlueprintReadWrite, transient, Category = Settings, meta = (PinShownByDefault))
	bool bShowDebug = false;

	// FAnimNode_Base interface
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
	virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;
	virtual bool HasPreUpdate() const override { return true; }
	virtual void PreUpdate(const UAnimInstance* InAnimInstance) override;
	// End of FAnimNode_Base interface

	// load deprecated properties
	bool Serialize(FArchive& Ar);
	void PostSerialize(const FArchive& Ar);

private:
	
	float TrackPosition = 0.0f;

	float GravityOffsetLen = 0.0f;
	float DistCamHeadAndNeck = 0.0f;

	ECraneJoints ColumnRotationBone = ECraneJoints::Columns;

	FTransform Target = FTransform::Identity;
	FVector RawRotation = FVector::ZeroVector;
	FQuat NeckQ = FQuat::Identity;

	// map between technocrane a name in skeleton and compact pose bone index and it's parent index
	TMap<ECraneJoints, TPair<FCompactPoseBoneIndex, FCompactPoseBoneIndex>>	CraneJointToCompactBoneIndex;

};

template<>
struct TStructOpsTypeTraits<FAnimNode_TechnocraneRig> : public TStructOpsTypeTraitsBase2<FAnimNode_TechnocraneRig>
{
	enum
	{
		WithSerializer = true,
		WithPostSerialize = true,
	};
};

