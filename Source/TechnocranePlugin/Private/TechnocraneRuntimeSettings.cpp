// Copyright (c) 2019 Technocrane s.r.o. 
//
// TechnocraneRuntimeSettings.cpp
// Sergei <Neill3d> Solokhin 2019

#include "TechnocraneRuntimeSettings.h"
#include "TechnocranePrivatePCH.h"

UTechnocraneRuntimeSettings::UTechnocraneRuntimeSettings()
{
	bLiveByDefault = false;
	PortIdByDefault = 1;
	SpaceScaleByDefault = 100.0f;
	bPacketContainsRawAndCalibratedData = false;
}