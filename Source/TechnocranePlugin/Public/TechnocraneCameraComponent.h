// Copyright (c) 2020-2024 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// TechnocraneCameraComponent.h
// Sergei <Neill3d> Solokhin

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CineCameraActor.h"
#include "CineCameraComponent.h"
#include "Misc/Timecode.h"

#include "TechnocraneCameraComponent.generated.h"

/// <summary>
/// A camera component class that exposes additional variables to align with Technocrane Trimmer exported camera data.
/// @sa ATDCamera
/// </summary>
UCLASS(ClassGroup = (Technocrane), meta = (DisplayName = "TDCamera Component"))
class TECHNOCRANEPLUGIN_API UTechnocraneCameraComponent : public UCineCameraComponent
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	UTechnocraneCameraComponent();
	virtual ~UTechnocraneCameraComponent();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracking Raw Data")
	float Zoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracking Raw Data")
	float Iris;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracking Raw Data")
	float Focus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracking Raw Data")
	float TrackPos;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracking Raw Data")
	float PacketNumber;

	//

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracking Calibrated")
	bool ApplyCalibratedValues = true;

	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Calibrated")
	bool IsZoomCalibrated;

	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Calibrated")
	bool IsIrisCalibrated;

	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Calibrated")
	bool IsFocusCalibrated;

	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Calibrated")
	float CalibratedZoom;

	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Calibrated")
	float CalibratedIris;

	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Calibrated")
	float CalibratedFocus;

	//

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Tracking Options")
		float SpaceScale;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Tracking Options")
		FFrameRate FrameRate;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Tracking Options")
		FVector2D ZoomRange;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Tracking Options")
		FVector2D IrisRange;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Tracking Options")
		FVector2D FocusRange;
};
