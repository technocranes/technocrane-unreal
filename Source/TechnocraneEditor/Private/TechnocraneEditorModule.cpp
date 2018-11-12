// Copyright (c) 2018 Technocrane s.r.o. 
//
// TechnocraneEditorModule.cpp
// Sergei <Neill3d> Solokhin 2018

#include "TechnocraneEditorPCH.h"
#include "TechnocraneEditorModule.h"
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

#define LOCTEXT_NAMESPACE "TechnocraneEditor"

IMPLEMENT_MODULE(FTechnocraneEditorModule, FTechnocraneEditor)

static EAssetTypeCategories::Type TechnocraneAssetCategoryBit;

/** All created asset type actions.  Cached here so that we can unregister them during shutdown. */
TArray< TSharedPtr<IAssetTypeActions> > CreatedAssetTypeActions;

void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
{
	AssetTools.RegisterAssetTypeActions(Action);
	CreatedAssetTypeActions.Add(Action);
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
		
		//@TODO: Struct registration should happen using ::StaticStruct, not by string!!!
		//PropertyModule.RegisterCustomPropertyTypeLayout( "SpritePolygonCollection", FOnGetPropertyTypeCustomizationInstance::CreateStatic( &FSpritePolygonCollectionCustomization::MakeInstance ) );

		PropertyModule.NotifyCustomizationModuleChanged();
	}
}

void FTechnocraneEditorModule::ShutdownModule()
{
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


