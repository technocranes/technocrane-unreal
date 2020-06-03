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
	DetailBuilder.EditCategory("Tracking Options", FText::GetEmpty(), ECategoryPriority::Important);
	DetailBuilder.EditCategory("Tracking Raw Data", FText::GetEmpty(), ECategoryPriority::Important);

	// Build the Connection category
	IDetailCategoryBuilder& ConnectionCategory = DetailBuilder.EditCategory("Connection");
	BuildConnectionSection(ConnectionCategory, DetailBuilder);

	// Build the Tracking Data category
	IDetailCategoryBuilder& TrackingDataCategory = DetailBuilder.EditCategory("Tracking Raw Data");
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

	Category.AddProperty(DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(ATechnocraneCamera, IsZoomCalibrated)))
		.DisplayName(LOCTEXT("IsZoomCalibrated", "Is Zoom Calibrated"))
		.Visibility(ShownInfo);
	Category.AddProperty(DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(ATechnocraneCamera, IsIrisCalibrated)))
		.DisplayName(LOCTEXT("IsIrisCalibrated", "Is Iris Calibrated"))
		.Visibility(ShownInfo);
	Category.AddProperty(DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(ATechnocraneCamera, IsFocusCalibrated)))
		.DisplayName(LOCTEXT("IsFocusCalibrated", "Is Focus Calibrated"))
		.Visibility(ShownInfo);
}

void FCameraDetailsCustomization::BuildConnectionSection(IDetailCategoryBuilder& Category, IDetailLayoutBuilder& DetailLayout)
{
	TSharedPtr<IPropertyHandle> NetworkConnectionDomainProperty = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(ATechnocraneCamera, UseNetworkConnection));
	
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

	TAttribute<EVisibility> ShownNetwork = TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &FCameraDetailsCustomization::GetCustomLiveVisibility, NetworkConnectionDomainProperty));
	
	Category.AddProperty(DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(ATechnocraneCamera, UseNetworkConnection)))
		.DisplayName(LOCTEXT("Use Network Connection", "Use Network Connection"));

	Category.AddProperty(DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(ATechnocraneCamera, SerialPort)))
		.DisplayName(LOCTEXT("Serial Port", "COM Port Id"));

	Category.AddProperty(DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(ATechnocraneCamera, NetworkBindAnyAddress)))
		.DisplayName(LOCTEXT("NetworkBindAnyAddress", "UDP Bind Any Address"))
		.Visibility(ShownNetwork);

	Category.AddProperty(DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(ATechnocraneCamera, NetworkBroadcast)))
		.DisplayName(LOCTEXT("NetworkBroadcast", "UDP Broadcast"))
		.Visibility(ShownNetwork);

	Category.AddProperty(DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(ATechnocraneCamera, NetworkAddress)))
		.DisplayName(LOCTEXT("NetworkAddress", "UDP Server Address"))
		.Visibility(ShownNetwork);

	Category.AddProperty(DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(ATechnocraneCamera, NetworkPort)))
		.DisplayName(LOCTEXT("Network Port", "UDP Port Id"))
		.Visibility(ShownNetwork);
}

FText FCameraDetailsCustomization::GetConnectionStatusText(TWeakObjectPtr<ATechnocraneCamera> WeakSprite) const
{
	FText HeaderDisplayText;
#if defined(TECHNOCRANESDK)
	if (ATechnocraneCamera* CameraBeingEdited = WeakSprite.Get())
	{
		const bool isReady = CameraBeingEdited->IsReady();
		const int last_error = CameraBeingEdited->GetLastError();

		if (isReady)
		{
			HeaderDisplayText = LOCTEXT("Status", "Connection Established, Tracking");
		}
		else
		{
			if (last_error == TECHNOCRANE_NO_ERROR)
			{
				HeaderDisplayText = LOCTEXT("Status", "Ready For Connection");
			}
			else if (last_error == TECHNOCRANE_NO_PORTS)
			{
				HeaderDisplayText = LOCTEXT("Status", TECHNOCRANE_NO_PORTS_MSG);
			}
			else if (last_error == TECHNOCRANE_SERIAL_FAILED)
			{
				HeaderDisplayText = LOCTEXT("Status", TECHNOCRANE_SERIAL_FAILED_MSG);
			}
			else
			{
				HeaderDisplayText = LOCTEXT("Status", "Unknown Connection Error");
			}
		}
	}
#endif
	return HeaderDisplayText;
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
