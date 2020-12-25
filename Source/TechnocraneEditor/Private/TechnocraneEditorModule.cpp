// Copyright (c) 2020 Technocrane s.r.o. 
//
// TechnocraneEditorModule.cpp
// Sergei <Neill3d> Solokhin

#include "TechnocraneEditorModule.h"
#include "TechnocraneEditorPCH.h"

#include "Stats/Stats.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "LevelEditor/Public/LevelEditor.h"
#include "LevelEditor/Private/SLevelEditor.h"
#include "LevelEditor/Public/SLevelViewport.h"
#include "AssetToolsModule.h"
#include "Factories/Factory.h"
#include "AssetTypeActions_Base.h"

#include "AssetToolsModule.h"
#include "PropertyEditorModule.h"
#include "IAssetTypeActions.h"

#include "CameraAssetTypeActions.h"
#include "CameraDetailsCustomization.h"

#include "TechnocraneCamera.h"
#include "TechnocraneEditorCommands.h"
#include "TechnocraneEditorStyle.h"

// Settings
#include "TechnocraneRuntimeSettings.h"
#include "ISettingsModule.h"

#define LOCTEXT_NAMESPACE "TechnocraneEditor"

IMPLEMENT_MODULE(FTechnocraneEditorModule, TechnocraneEditor)

static EAssetTypeCategories::Type TechnocraneAssetCategoryBit;

/** All created asset type actions.  Cached here so that we can unregister them during shutdown. */
TArray< TSharedPtr<IAssetTypeActions> > CreatedAssetTypeActions;

void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
{
	AssetTools.RegisterAssetTypeActions(Action);
	CreatedAssetTypeActions.Add(Action);
}

void RegisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "TechnocranePlugin",
			LOCTEXT("RuntimeSettingsName", "Technocrane Tracker"),
			LOCTEXT("RuntimeSettingsDescription", "Configure the Technocrane plugin"),
			GetMutableDefault<UTechnocraneRuntimeSettings>());
	}
}

void UnregisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "TechnocranePlugin");
	}
}

void FTechnocraneEditorModule::StartupModule()
{
	// Register the new assets
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

		TechnocraneAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("Technocrane")), LOCTEXT("TechnocraneAssetCategory", "Technocrane"));

		RegisterAssetTypeAction(AssetTools, MakeShareable(new FAssetTypeActions_TechnocraneCamera(TechnocraneAssetCategoryBit)));

	}

	// Register the details customizations
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.RegisterCustomClassLayout(ATechnocraneCamera::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FCameraDetailsCustomization::MakeInstance));
		
		PropertyModule.NotifyCustomizationModuleChanged();
	}

	// Settings
	RegisterSettings();

	//
	FTechnocraneEditorStyle::Initialize();
	FTechnocraneEditorStyle::ReloadTextures();

	FTechnocraneEditorCommands::Register();

	// Toolbar
	PluginCommands = MakeShareable(new FUICommandList);

	// Dummy action for main toolbar button
	PluginCommands->MapAction(
		FTechnocraneEditorCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FTechnocraneEditorModule::PluginButtonClicked),
		FCanExecuteAction());

	// TODO: add functionality and activate the toolbar button
	// Only add the toolbar if TechnocranePlugin is the currently active
	//static FName SystemName(TEXT("Technocrane"));
	//if (FModuleManager::Get().IsModuleLoaded("TechnocranePlugin"))
	//{
	//	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
	//	ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FTechnocraneEditorModule::AddToolbarExtension));
	//
	//	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	//	LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	//}
}

void FTechnocraneEditorModule::PluginButtonClicked()
{
	// Empty on purpose
}

void FTechnocraneEditorModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddComboButton(
		FUIAction(FExecuteAction::CreateRaw(this, &FTechnocraneEditorModule::PluginButtonClicked)),
		FOnGetContent::CreateRaw(this, &FTechnocraneEditorModule::FillComboButton, PluginCommands),
		LOCTEXT("TechnocraneInputBtn", "Technocrane"),
		LOCTEXT("TechnocraneInputBtnTootlip", "Technocrane"),
		FSlateIcon(FTechnocraneEditorStyle::GetStyleSetName(), "TechnocraneEditor.PluginAction")
	);
}

TSharedRef<SWidget> FTechnocraneEditorModule::FillComboButton(TSharedPtr<class FUICommandList> Commands)
{
	FMenuBuilder MenuBuilder(true, Commands);

	return MenuBuilder.MakeWidget();
}

void FTechnocraneEditorModule::ShutdownModule()
{
	UnregisterSettings();

	// Unregister all the asset types that we registered
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (int32 Index = 0; Index < CreatedAssetTypeActions.Num(); ++Index)
		{
			AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions[Index].ToSharedRef());
		}
	}
	CreatedAssetTypeActions.Empty();
}


