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

UCLASS(Blueprintable, ClassGroup = Technocrane, Category = "Technocrane", meta = (BlueprintSpawnableComponent))
class TECHNOCRANEPLUGIN_API ATechnocraneCamera : public ACineCameraActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATechnocraneCamera(const FObjectInitializer& ObjectInitializer);
	virtual ~ATechnocraneCamera();

	const bool IsReady() const {
		return mHardware->IsReady();
	}

	const int GetLastError() const {
		return mHardware->GetLastError();
	}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Controls the amount of rails for the crane rig. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Connection, meta = (ClampMin = 0, ClampMax = 10))
		int Port;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = Connection)
		bool Live;

	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Raw Data")
		float TrackPosition;
	
	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Raw Data")
		FVector RawPosition;

	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Raw Data")
		FVector RawRotation;

	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Raw Data")
		bool HasTimeCode;

	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Raw Data")
		FVector4 TimeCode;

	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Raw Data")
		int32 PacketNumber;

	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Raw Data")
		bool IsZoomCalibrated;

	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Raw Data")
		bool IsIrisCalibrated;

	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Raw Data")
		bool IsFocusCalibrated;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Tracking Options")
		float SpaceScale;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Tracking Options")
		FVector2D ZoomRange;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Tracking Options")
		FVector2D IrisRange;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Tracking Options")
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
