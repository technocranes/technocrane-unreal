// Copyright (c) 2025 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// TechnocraneRigAnimInstance.cpp
// Sergei <Neill3d> Solokhin

#include "TechnocraneRigAnimInstance.h"
#include "TechnocraneRigInstanceProxy.h"

///////////////////////////////////
/// Anim Instance 
///////////////////////////////////

void UTechnocraneRigAnimInstance::NativeInitializeAnimation()
{
	FTechnocraneRigInstanceProxy& Proxy = GetProxyOnGameThread<FTechnocraneRigInstanceProxy>();
	Proxy.Initialize(this);
}

void UTechnocraneRigAnimInstance::ConfigureAnimInstance(TWeakObjectPtr<AActor> InTargetActor, FCraneData& InCraneData, const FVector& InCameraPivotOffset, bool bShowDebug)
{
	FTechnocraneRigInstanceProxy& Proxy = GetProxyOnGameThread<FTechnocraneRigInstanceProxy>();
	Proxy.ConfigureAnimInstanceProxy(InTargetActor, InCraneData, InCameraPivotOffset, bShowDebug);
}

FAnimInstanceProxy* UTechnocraneRigAnimInstance::CreateAnimInstanceProxy()
{
	return new FTechnocraneRigInstanceProxy(this, &AnimNode);
}
