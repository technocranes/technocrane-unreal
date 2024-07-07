// Copyright (c) 2020-2024 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// TechnocraneCamera.h
// Sergei <Neill3d> Solokhin

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <Runtime/CinematicCamera/Public/CineCameraActor.h>
#include "Misc/Timecode.h"

#include "TechnocraneCamera.generated.h"

// forward
class UTechnocraneCameraComponent;

/// <summary>
/// A version of CineCamera that is using a @sa TechnocraneCameraComponent
///  The component exposes properties that match those stored in Technocrane Trimmer FBX with camera data export.
/// </summary>
UCLASS(Blueprintable, ClassGroup = Technocrane, Category = "Technocrane", meta = (DisplayName = "Technocrane Camera", BlueprintSpawnableComponent))
class TECHNOCRANEPLUGIN_API ATDCamera : public ACineCameraActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATDCamera(const FObjectInitializer& ObjectInitializer);
	
	UTechnocraneCameraComponent* GetTechnocraneCameraComponent() const { return TechnocraneCameraComponent; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	/** Returns TechnocraneCameraComponent subobject **/
	class UTechnocraneCameraComponent* TechnocraneCameraComponent;
};
