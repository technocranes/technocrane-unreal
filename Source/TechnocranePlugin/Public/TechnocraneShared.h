#pragma once

// Copyright (c) 2018 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// TechnocraneShared.h
// Sergei <Neill3d> Solokhin 2018

#include "CoreMinimal.h"

#define TECHNOCRANE_MAX_COLUMNS_COUNT	3
#define TECHNOCRANE_MAX_BEAMS_COUNT		4

UENUM( BlueprintType )
enum class ECraneJoints : uint8
{
	Base = 0,
	Columns,
	Column1,
	Column2,
	Column3,
	Beams,
	Beam1,
	Beam2,
	Beam3,
	Beam4,
	Gravity,
	Neck,
	Head,

	WheelFR,
	WheelFL,
	WheelRR,
	WheelRL,

	JointCount
};

const FName GetCraneJointName(const ECraneJoints index);