// Copyright (c) 2018 Technocrane s.r.o. 
//
// CameraDetailsCustomization.cpp
// Sergei <Neill3d> Solokhin 2018

#include "TechnocraneEditorPCH.h"
#include "CameraDetailsCustomization.h"
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

#define LOCTEXT_NAMESPACE "TechnocraneEditor"

//////////////////////////////////////////////////////////////////////////
// FFlipbookComponentDetailsCustomization

TSharedRef<IDetailCustomization> FCameraDetailsCustomization::MakeInstance()
{
	return MakeShareable(new FCameraDetailsCustomization);
}

void FCameraDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Create a category so this is displayed early in the properties
	DetailBuilder.EditCategory("Connection", FText::GetEmpty(), ECategoryPriority::Important);
	DetailBuilder.EditCategory("Tracking Calibration", FText::GetEmpty(), ECategoryPriority::Important);
	DetailBuilder.EditCategory("Raw Tracking Data", FText::GetEmpty(), ECategoryPriority::Important);

	// Build the Connection category
	IDetailCategoryBuilder& ConnectionCategory = DetailBuilder.EditCategory("Connection");
	BuildConnectionSection(ConnectionCategory, DetailBuilder);

	// Build the Tracking Data category
	IDetailCategoryBuilder& TrackingDataCategory = DetailBuilder.EditCategory("Tracking Data");
	BuildTrackingDataSection(TrackingDataCategory, DetailBuilder);
}

EVisibility FCameraDetailsCustomization::GetCustomLiveVisibility(TSharedPtr<IPropertyHandle> Property) const
{
	if (Property.IsValid())
	{
		bool ValueAsBool;
		FPropertyAccess::Result Result = Property->GetValue(/*out*/ ValueAsBool);

		if (Result == FPropertyAccess::Success)
		{
			return (ValueAsBool == true) ? EVisibility::Visible : EVisibility::Collapsed;
		}
	}

	// If there are multiple values, show all properties
	return EVisibility::Visible;
}

void FCameraDetailsCustomization::BuildTrackingDataSection(IDetailCategoryBuilder& Category, IDetailLayoutBuilder& DetailLayout)
{
	TSharedPtr<IPropertyHandle> CameraLiveDomainProperty = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(ATechnocraneCamera, Live));


	static const FText TrackingInfoInLiveMode = LOCTEXT("TrackingPropertiesShownInLiveMode", "Switch to 'Live' mode\nto see tracking details");
	Category.AddCustomRow(TrackingInfoInLiveMode)
		.WholeRowContent()
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Font(DetailLayout.GetDetailFontItalic())
		.Justification(ETextJustify::Center)
		.Text(TrackingInfoInLiveMode)
		];

	TAttribute<EVisibility> ShownInfo = TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &FCameraDetailsCustomization::GetCustomLiveVisibility, CameraLiveDomainProperty));

	Category.AddProperty(DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(ATechnocraneCamera, TrackPosition)))
		.DisplayName(LOCTEXT("TrackPosition", "Packet Track Position"))
		.Visibility(ShownInfo);
	Category.AddProperty(DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(ATechnocraneCamera, RawPosition)))
		.DisplayName(LOCTEXT("RawPosition", "Packet Raw Position"))
		.Visibility(ShownInfo);
	Category.AddProperty(DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(ATechnocraneCamera, RawRotation)))
		.DisplayName(LOCTEXT("RawRotation", "Packet Raw Rotation"))
		.Visibility(ShownInfo);

	Category.AddProperty(DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(ATechnocraneCamera, HasTimeCode)))
		.DisplayName(LOCTEXT("HasTimeCode", "Packet Contains A Time Code"))
		.Visibility(ShownInfo);
	Category.AddProperty(DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(ATechnocraneCamera, TimeCode)))
		.DisplayName(LOCTEXT("TimeCode", "Packet Time Code"))
		.Visibility(ShownInfo);
	Category.AddProperty(DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(ATechnocraneCamera, PacketNumber)))
		.DisplayName(LOCTEXT("PacketNumber", "Packet Number"))
		.Visibility(ShownInfo);
}

void FCameraDetailsCustomization::BuildConnectionSection(IDetailCategoryBuilder& Category, IDetailLayoutBuilder& DetailLayout)
{
	
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailLayout.GetObjectsBeingCustomized(/*out*/ ObjectsBeingCustomized);
	
	if (ObjectsBeingCustomized.Num() > 0)
	{
		if (ATechnocraneCamera* camera = Cast<ATechnocraneCamera>(ObjectsBeingCustomized[0].Get()))
		{
			static const FText TypesOfMaterialsTooltip = LOCTEXT("ConnectionStatus", "This is a current communication status with a Technocrane device");

			Category.AddCustomRow(TypesOfMaterialsTooltip)
				.WholeRowContent()
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Font(DetailLayout.GetDetailFontItalic())
				.Justification(ETextJustify::Center)
				.Text(this, &FCameraDetailsCustomization::GetConnectionStatusText, MakeWeakObjectPtr(camera))
				];

		}
	}

	Category.AddProperty(DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(ATechnocraneCamera, Port)))
		.DisplayName(LOCTEXT("Port", "COM Port Id"));
}

FText FCameraDetailsCustomization::GetConnectionStatusText(TWeakObjectPtr<ATechnocraneCamera> WeakSprite) const
{
	FText HeaderDisplayText;

	if (ATechnocraneCamera* CameraBeingEdited = WeakSprite.Get())
	{
		HeaderDisplayText = LOCTEXT("Status", "Ready For Connection");
	}

	return HeaderDisplayText;
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
