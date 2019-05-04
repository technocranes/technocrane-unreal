// Copyright (c) 2019 Technocrane s.r.o. 
// 
// TechnocraneShared.cpp
// Sergei <Neill3d> Solokhin 2019

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