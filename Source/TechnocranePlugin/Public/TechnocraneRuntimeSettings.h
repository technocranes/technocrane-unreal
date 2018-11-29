// Copyright (c) 2018 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// TechnocraneRuntimeSettings.h
// Sergei <Neill3d> Solokhin 2018

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "TechnocraneRuntimeSettings.generated.h"

/**
 * Implements the settings for the Paper2D plugin.
 */
UCLASS(config=Engine, defaultconfig)
class TECHNOCRANEPLUGIN_API UTechnocraneRuntimeSettings : public UObject
{
	GENERATED_BODY()
public:
	//! a constructor
	UTechnocraneRuntimeSettings();

	// Specify a default state of live mode for a Technocrane Camera Connection
	UPROPERTY(EditAnywhere, config, Category=Settings)
	bool bLiveByDefault;

	// Specify a default port index for a Technocrane Camera
	UPROPERTY(EditAnywhere, config, Category = Settings)
	int PortIdByDefault;

	// Specify a default packet raw data space scale
	UPROPERTY(EditAnywhere, config, Category=Settings)
	float SpaceScaleByDefault;
	
	// Specify a default packet raw data space scale
	UPROPERTY(EditAnywhere, config, Category = Settings)
	bool bPacketContainsRawAndCalibratedData;
};
