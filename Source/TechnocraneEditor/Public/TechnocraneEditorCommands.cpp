// Copyright (c) 2020 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// TechnocraneEditorCommands.cpp
// Sergei <Neill3d> Solokhin 2020

#include "TechnocraneEditorCommands.h"


void FTechnocraneEditorCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "TechnocraneEditor", "Add Tracker to a camera", EUserInterfaceActionType::Button, FInputGesture());
}

