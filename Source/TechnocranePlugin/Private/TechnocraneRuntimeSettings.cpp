// Copyright (c) 2020 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// TechnocraneRuntimeSettings.cpp
// Sergei <Neill3d> Solokhin

#include "TechnocraneRuntimeSettings.h"
#include "TechnocranePrivatePCH.h"

#include "Math\Interval.h"

UTechnocraneRuntimeSettings::UTechnocraneRuntimeSettings()
	: CameraFrameRate(25, 1)
{
	bLiveByDefault = false;
	bNetworkConnection = false;
	SerialPortIdByDefault = 1;
	bNetworkBindAnyAddress = true;
	bNetworkBroadcast = false;
	NetworkServerAddressByDefault = "127.0.0.1"; // localhost
	NetworkPortIdByDefault = 15246;
	SpaceScaleByDefault = 100.0f;
	bPacketContainsRawAndCalibratedData = false;

	ZoomRange = FFloatInterval(0.0f, 100.0f);
	FocusRange = FFloatInterval(0.0f, 100.0f);
	IrisRange = FFloatInterval(0.0f, 100.0f);
}