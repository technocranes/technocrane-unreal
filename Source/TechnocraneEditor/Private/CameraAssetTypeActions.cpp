// Copyright (c) 2020 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// CameraAssetTypeActions.cpp
// Sergei <Neill3d> Solokhin

#include "CameraAssetTypeActions.h"
#include "TechnocraneEditorPCH.h"

#include "TechnocraneCamera.h"

#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

//////////////////////////////////////////////////////////////////////////
// FFlipbookAssetTypeActions

FAssetTypeActions_TechnocraneCamera::FAssetTypeActions_TechnocraneCamera(EAssetTypeCategories::Type InAssetCategory)
	: MyAssetCategory(InAssetCategory)
{
}

FText FAssetTypeActions_TechnocraneCamera::GetName() const
{
	return LOCTEXT("FCameraAssetTypeActionsName", "Technocrane Camera");
}

FColor FAssetTypeActions_TechnocraneCamera::GetTypeColor() const
{
	return FColor(129, 196, 115);
}

UClass* FAssetTypeActions_TechnocraneCamera::GetSupportedClass() const
{
	return ATechnocraneCamera::StaticClass();
}

uint32 FAssetTypeActions_TechnocraneCamera::GetCategories()
{
	return MyAssetCategory; // EAssetTypeCategories::Animation | 
}

//////////////////////////////////////////////////////////////////////////

UTechnocraneCameraFactory::UTechnocraneCameraFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = ATechnocraneCamera::StaticClass();

	bCreateNew = true;
	bEditorImport = false;
	bEditAfterNew = true;
}

UObject* UTechnocraneCameraFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	ATechnocraneCamera* NewPreset = NewObject<ATechnocraneCamera>(InParent, InName, Flags);

	return NewPreset;
}

#undef LOCTEXT_NAMESPACE
