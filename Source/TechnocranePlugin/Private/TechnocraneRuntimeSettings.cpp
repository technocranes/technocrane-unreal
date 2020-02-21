// Copyright (c) 2019 Technocrane s.r.o. 
//
// TechnocraneRuntimeSettings.cpp
// Sergei <Neill3d> Solokhin 2019

#include "TechnocraneRuntimeSettings.h"
#include "TechnocranePrivatePCH.h"

UTechnocraneRuntimeSettings::UTechnocraneRuntimeSettings()
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
}