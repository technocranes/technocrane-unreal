// Copyright (c) 2018 Technocrane s.r.o. 
//
// CameraAssetTypeActions.h
// Sergei <Neill3d> Solokhin 2018
#pragma once

#include "CoreMinimal.h"
#include "Stats/Stats.h"
#include "AssetToolsModule.h"
#include "Factories/Factory.h"
#include "AssetTypeActions_Base.h"

#include "CameraAssetTypeActions.generated.h"

class FAssetTypeActions_TechnocraneCamera : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_TechnocraneCamera(EAssetTypeCategories::Type InAssetCategory);

	// IAssetTypeActions interface
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override
	{}
	// End of IAssetTypeActions interface

private:
	EAssetTypeCategories::Type MyAssetCategory;
};

UCLASS(MinimalAPI, hidecategories = Object)
class UTechnocraneCameraFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

		//~ Begin UFactory Interface
		virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	//~ Begin UFactory Interface	
};