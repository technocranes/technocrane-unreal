// Copyright (c) 2025 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// TechnocraneRigAnimInstance.h
// Sergei <Neill3d> Solokhin

#pragma once

#include "Animation/AnimInstance.h"
#include "Animation/AnimNodeBase.h"
#include "AnimNode_TechnocraneRig.h"

#include "TechnocraneRigAnimInstance.generated.h"

class UIKRetargeter;
class USkeletalMeshComponent;
struct FAnimNode_TechnocraneRig;
struct FTechnocraneRigInstanceProxy;

///////////////////////////
/// Anim Instance 
///////////////////////////

UCLASS(Transient, NotBlueprintable, BlueprintType, MinimalAPI)
class UTechnocraneRigAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

	friend FTechnocraneRigInstanceProxy;

public:
	/**
	* Configure TechnocraneRig AnimInstance
	* @param InTargetActor the actor which location will be used as a target for the crane simulation.
	*/
	void ConfigureAnimInstance(TWeakObjectPtr<AActor> InTargetActor, FCraneData& InCraneData, const FVector& InCameraPivotOffset, bool bShowDebug);
	
protected:
	/** UAnimInstance interface */
	virtual void NativeInitializeAnimation() override;
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
	/** UAnimInstance interface end*/
	
	UPROPERTY()
	FAnimNode_TechnocraneRig AnimNode;
};
