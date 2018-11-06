// Copyright (c) 2018 Technocrane s.r.o. 
//
// TechnocraneCamera.h
// Sergei <Neill3d> Solokhin 2018

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <Runtime/CinematicCamera/Public/CineCameraActor.h>
#include <../ThirdParty/TechnocraneSDK/include/technocrane_hardware.h>

#include "TechnocraneCamera.generated.h"

UCLASS(Blueprintable, ClassGroup = "Technocrane", meta = (BlueprintSpawnableComponent))
class ATechnocraneCamera : public ACineCameraActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATechnocraneCamera(const FObjectInitializer& ObjectInitializer);
	virtual ~ATechnocraneCamera();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Controls the amount of rails for the crane rig. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection", meta = (ClampMin = 0, ClampMax = 10))
		int Port;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Connection")
		bool Live;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Connection")
		float SpaceScale;

	UPROPERTY(SimpleDisplay, NoClear, BlueprintReadOnly, Category = "Tracking Data")
		float TrackPosition;
	
	UPROPERTY(SimpleDisplay, NoClear, BlueprintReadOnly, Category = "Tracking Data")
		FVector RawPosition;

	UPROPERTY(SimpleDisplay, NoClear, BlueprintReadOnly, Category = "Tracking Data")
		FVector RawRotation;

	UPROPERTY(SimpleDisplay, NoClear, BlueprintReadOnly, Category = "Tracking Data")
		bool HasTimeCode;

	UPROPERTY(SimpleDisplay, NoClear, BlueprintReadOnly, Category = "Tracking Data")
		FVector4 TimeCode;

	UPROPERTY(SimpleDisplay, NoClear, BlueprintReadOnly, Category = "Tracking Data")
		int32 PacketNumber;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Manual Calibration")
		FVector2D ZoomRange;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Manual Calibration")
		FVector2D IrisRange;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Manual Calibration")
		FVector2D FocusRange;

#if WITH_EDITOR
	virtual bool CanEditChange(const UProperty* InProperty) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditUndo() override;
#endif

protected:

	bool SwitchLive();

	NTechnocrane::CTechnocrane_Hardware	*	mHardware;
};
