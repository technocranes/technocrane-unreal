// Copyright (c) 2020 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// TechnocraneEditorModule.h
// Sergei <Neill3d> Solokhin

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class SWidget;

class FTechnocraneEditorModule : public IModuleInterface
{
public:

	// IModuleInterface interface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

protected:

	void PluginButtonClicked();

	void AddToolbarExtension(FToolBarBuilder& Builder);

	TSharedPtr<class FUICommandList> PluginCommands;
	TSharedRef<SWidget> FillComboButton(TSharedPtr<class FUICommandList> Commands);
};
