// Copyright (c) 2025 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// TechnocraneRigInstanceProxy.h
// Sergei <Neill3d> Solokhin
#pragma once

#include "TechnocraneRigAnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "TechnocraneData.h"
#include "TechnocraneRigInstanceProxy.generated.h"

class USkeletalMeshComponent;
struct FAnimNode_TechnocraneRig;

////////////////////////////////////
/// Anim Instance Proxy
////////////////////////////////////
USTRUCT()
struct FTechnocraneRigInstanceProxy : public FAnimInstanceProxy
{
public:
	
	GENERATED_BODY()

	FTechnocraneRigInstanceProxy() = default; //Constructor
	
	FTechnocraneRigInstanceProxy(UAnimInstance* InAnimInstance, FAnimNode_TechnocraneRig* InAnimNode);
	
public:	
	/* FAnimInstanceProxy Instance */ 
	virtual void Initialize(UAnimInstance* InAnimInstance) override; 
	virtual void PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds) override;
	virtual void CacheBones() override;
	virtual bool Evaluate(FPoseContext& Output) override;
	virtual void UpdateAnimationNode(const FAnimationUpdateContext& InContext) override;
	/* END FAnimInstanceProxy Instance */
	
	void ConfigureAnimInstanceProxy(TWeakObjectPtr<AActor> InTargetActor, FCraneData& InCraneData, const FVector& InCameraPivotOffset, bool bShowDebug);

	FAnimNode_TechnocraneRig* AnimNode = nullptr;
};
