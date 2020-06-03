// Copyright (c) 2020 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// TechnocraneEditorCommands.h
// Sergei <Neill3d> Solokhin

#pragma once

class FTechnocraneEditorCommands : public TCommands<FTechnocraneEditorCommands>
{
public:

	FTechnocraneEditorCommands()
		: TCommands<FTechnocraneEditorCommands>(TEXT("TechnocraneEditor"), NSLOCTEXT("Contexts", "TechncraneEditor", "TechnocraneEditor Plugin"), NAME_None, TEXT("TechnocraneEditor.Common.Icon"))
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> PluginAction;
};
