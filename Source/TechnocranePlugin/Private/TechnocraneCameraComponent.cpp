// Copyright (c) 2020-24 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// TechnocraneCameraComponent.cpp
// Sergei <Neill3d> Solokhin

#include "TechnocraneCameraComponent.h"
#include "TechnocranePrivatePCH.h"
#include "TechnocraneRuntimeSettings.h"
#include "ITechnocranePlugin.h"
#include <technocrane_hardware.h>

// Sets default values
UTechnocraneCameraComponent::UTechnocraneCameraComponent()
	: Super()
	, FrameRate(25, 1)
{
#if (ENGINE_MAJOR_VERSION > 4 || ENGINE_MINOR_VERSION >= 25)
		FocusSettings.FocusMethod = ECameraFocusMethod::Tracking;
#else
		FocusSettings.FocusMethod = ECameraFocusMethod::None;
#endif
		SetFilmbackPresetByName("35mm Full Aperture");
	
	SpaceScale = GetDefault<UTechnocraneRuntimeSettings>()->SpaceScaleByDefault;
	TrackPos = 0.0f;

	//
	ZoomRange = FVector2D(0.0f, 100.0f);
	IrisRange = FVector2D(0.0f, 100.0f);
	FocusRange = FVector2D(0.0f, 100.0f);
}

UTechnocraneCameraComponent::~UTechnocraneCameraComponent()
{
}

// Called every frame
void UTechnocraneCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	IsZoomCalibrated = NTechnocrane::ComputeZoomf(CalibratedZoom, Zoom, ZoomRange.X, ZoomRange.Y);
	IsIrisCalibrated = NTechnocrane::ComputeIrisf(CalibratedIris, Iris, IrisRange.X, IrisRange.Y);
	IsFocusCalibrated = NTechnocrane::ComputeFocusf(CalibratedFocus, Focus, FocusRange.X, FocusRange.Y);

	if (ApplyCalibratedValues)
	{
		CurrentFocalLength = CalibratedZoom;
		CurrentAperture = CalibratedIris;
		FocusSettings.ManualFocusDistance = SpaceScale * CalibratedFocus;
	}
	
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

