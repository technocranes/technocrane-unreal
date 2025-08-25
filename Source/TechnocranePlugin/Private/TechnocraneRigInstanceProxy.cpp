// Copyright (c) 2025 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// TechnocraneRigInstanceProxy.cpp
// Sergei <Neill3d> Solokhin
#include "TechnocraneRigInstanceProxy.h"
#include <Runtime/CinematicCamera/Public/CineCameraActor.h>

#include UE_INLINE_GENERATED_CPP_BY_NAME(TechnocraneRigInstanceProxy)


/////////////////////////////////
/// Anim instance proxy struct
/////////////////////////////////

FTechnocraneRigInstanceProxy::FTechnocraneRigInstanceProxy(UAnimInstance* InAnimInstance, FAnimNode_TechnocraneRig* InAnimNode)
	: FAnimInstanceProxy(InAnimInstance),
	AnimNode(InAnimNode)
{
}

void FTechnocraneRigInstanceProxy::Initialize(UAnimInstance* InAnimInstance)
{
	FAnimInstanceProxy::Initialize(InAnimInstance);

	//Initialize instance manually
	FAnimationInitializeContext InitContext(this);
	AnimNode->Initialize_AnyThread(InitContext);
}

void FTechnocraneRigInstanceProxy::PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds)
{
	if(InAnimInstance)
	{
		Super::PreUpdate(InAnimInstance, DeltaSeconds);
				
		if (AnimNode->HasPreUpdate())
		{
			AnimNode->PreUpdate(InAnimInstance);
		}
	}
}

// Cache bones if they are marked as invalid//
void FTechnocraneRigInstanceProxy::CacheBones()
{
	if(bBoneCachesInvalidated)
	{
		FAnimationCacheBonesContext Context(this);
		AnimNode->CacheBones_AnyThread(Context);
		bBoneCachesInvalidated = false;
	}
}

// Evaluate pose//
bool FTechnocraneRigInstanceProxy::Evaluate(FPoseContext& Output)
{
	Super::Evaluate(Output);
	
	/* This evaluates UAnimNode_TechnocraneRig */
	AnimNode->Evaluate_AnyThread(Output);
	return true;
}

//Update animation//
void FTechnocraneRigInstanceProxy::UpdateAnimationNode(const FAnimationUpdateContext& InContext)
{
	Super::UpdateAnimationNode(InContext);

	UpdateCounter.Increment();
	
	AnimNode->Update_AnyThread(InContext);
}

void FTechnocraneRigInstanceProxy::ConfigureAnimInstanceProxy(TWeakObjectPtr<AActor> InTargetActor, FCraneData& InCraneData, const FVector& InCameraPivotOffset, bool bShowDebug)
{
	if (ACineCameraActor* CineCamera = Cast<ACineCameraActor>(InTargetActor))
	{
		AnimNode->TargetCameraActor = CineCamera;
	}
	AnimNode->CraneData = InCraneData;
	AnimNode->CameraPivotOffset = InCameraPivotOffset;
	AnimNode->bShowDebug = bShowDebug;
}
