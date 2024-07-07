// Copyright (c) 2020-2024 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// TechnocraneCamera.cpp
// Sergei <Neill3d> Solokhin

#include "TechnocraneCamera.h"
#include "TechnocraneCameraComponent.h"
#include "TechnocranePrivatePCH.h"
#include "TechnocraneRuntimeSettings.h"
#include "ITechnocranePlugin.h"
#include <Runtime/CinematicCamera/Public/CineCameraComponent.h>
#include <technocrane_hardware.h>

// Sets default values
ATDCamera::ATDCamera(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer
		.SetDefaultSubobjectClass<UTechnocraneCameraComponent>(TEXT("CameraComponent")))
	
{
	TechnocraneCameraComponent = Cast<UTechnocraneCameraComponent>(GetCameraComponent());

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

// Called when the game starts or when spawned
void ATDCamera::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATDCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

