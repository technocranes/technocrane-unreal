// Copyright (c) 2019 Technocrane s.r.o. 
//
// CameraDetailsCustomization.h
// Sergei <Neill3d> Solokhin 2019

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IDetailCategoryBuilder;
class IDetailLayoutBuilder;
class ATechnocraneCamera;

//////////////////////////////////////////////////////////////////////////
// FFlipbookComponentDetailsCustomization

class FCameraDetailsCustomization : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
	// End of IDetailCustomization interface

protected:
	void BuildConnectionSection(IDetailCategoryBuilder& RenderingCategory, IDetailLayoutBuilder& DetailLayout);
	void BuildTrackingDataSection(IDetailCategoryBuilder& category, IDetailLayoutBuilder& DetailLayout);

	FText GetConnectionStatusText(TWeakObjectPtr<ATechnocraneCamera> WeakSprite) const;
	EVisibility GetCustomLiveVisibility(TSharedPtr<IPropertyHandle> Property) const;
};
