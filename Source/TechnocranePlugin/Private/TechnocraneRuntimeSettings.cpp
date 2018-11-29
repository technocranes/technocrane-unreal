// Copyright (c) 2018 Technocrane s.r.o. 
//
// TechnocraneRuntimeSettings.cpp
// Sergei <Neill3d> Solokhin 2018

#include "TechnocranePrivatePCH.h"
#include "TechnocraneRuntimeSettings.h"

UTechnocraneRuntimeSettings::UTechnocraneRuntimeSettings()
{
	bLiveByDefault = false;
	PortIdByDefault = 1;
	SpaceScaleByDefault = 100.0f;
	bPacketContainsRawAndCalibratedData = false;
}