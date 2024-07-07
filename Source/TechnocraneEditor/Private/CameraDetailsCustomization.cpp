// Copyright (c) 2020 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// CameraDetailsCustomization.cpp
// Sergei <Neill3d> Solokhin

#include "CameraDetailsCustomization.h"
#include "TechnocraneEditorPCH.h"

#include "DetailLayoutBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailPropertyRow.h"
#include "DetailCategoryBuilder.h"
#include "PropertyCustomizationHelpers.h"

#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SToolTip.h"

#include "TechnocraneCamera.h"
#include <ThirdParty/TechnocraneSDK/include/technocrane_hardware.h>

#define LOCTEXT_NAMESPACE "TechnocraneCamera"

//////////////////////////////////////////////////////////////////////////
// FFlipbookComponentDetailsCustomization

TSharedRef<IDetailCustomization> FCameraDetailsCustomization::MakeInstance()
{
	return MakeShareable(new FCameraDetailsCustomization);
}

void FCameraDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Create a category so this is displayed early in the properties
	DetailBuilder.EditCategory("Tracking Raw Data", FText::GetEmpty(), ECategoryPriority::Important);
	DetailBuilder.EditCategory("Tracking Calibrated", FText::GetEmpty(), ECategoryPriority::Important);
	DetailBuilder.EditCategory("Tracking Options", FText::GetEmpty(), ECategoryPriority::Important);
}


//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
