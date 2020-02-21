// Copyright (c) 2019 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// TechnocraneRuntimeSettings.h
// Sergei <Neill3d> Solokhin 2019

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

	// switch between serial or network connections
	UPROPERTY(EditAnywhere, config, Category = Settings)
	bool bNetworkConnection;

	// Specify a default serial port index for a Technocrane Camera
	UPROPERTY(EditAnywhere, config, Category = Settings)
	int SerialPortIdByDefault;

	// try to catch packets from a specified udp port broadcasted into a local network
	UPROPERTY(EditAnywhere, config, Category = NetworkSettings)
	bool bNetworkBindAnyAddress;

	UPROPERTY(EditAnywhere, config, Category = NetworkSettings)
	bool bNetworkBroadcast;

	// Specify a default network server address
	UPROPERTY(EditAnywhere, config, Category = NetworkSettings)
	FString NetworkServerAddressByDefault;
	
	// Specify a default network port index for a Technocrane Camera
	UPROPERTY(EditAnywhere, config, Category = NetworkSettings)
	int NetworkPortIdByDefault;

	// Specify a default packet raw data space scale
	UPROPERTY(EditAnywhere, config, Category=Settings)
	float SpaceScaleByDefault;
	
	// Specify a default packet raw data space scale
	UPROPERTY(EditAnywhere, config, Category = Settings)
	bool bPacketContainsRawAndCalibratedData;
};
