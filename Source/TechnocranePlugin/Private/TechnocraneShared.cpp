// Copyright (c) 2020 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// TechnocraneShared.cpp
// Sergei <Neill3d> Solokhin

#include "TechnocraneShared.h"
#include "TechnocranePrivatePCH.h"


const char* g_CraneJointNames[static_cast<int32>(ECraneJoints::JointCount)] = {
	"jointRoot",
	"jointColumns",
	"jointColumn1",
	"jointColumn2",
	"jointColumn3",
	"jointBeams",
	"jointBeam1",
	"jointBeam2",
	"jointBeam3",
	"jointBeam4",
	"jointBeam5",
	"jointGravity",
	"jointNeck",
	"jointHead",

	"jointWheelFR",
	"jointWheelFL",
	"jointWheelRR",
	"jointWheelRL",
};

const FName GetCraneJointName(const ECraneJoints enum_index)
{
	const int32 index = static_cast<int32>(enum_index);
	return g_CraneJointNames[index];
}