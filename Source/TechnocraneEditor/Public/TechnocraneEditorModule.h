// Copyright (c) 2018 Technocrane s.r.o. 
//
// TechnocraneEditorModule.h
// Sergei <Neill3d> Solokhin 2018

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class FTechnocraneEditorModule : public IModuleInterface
{
public:

	// IModuleInterface interface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
