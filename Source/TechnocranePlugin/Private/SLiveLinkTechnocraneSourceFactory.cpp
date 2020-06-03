// Copyright (c) 2020 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// SLiveLinkTechnocraneSourceFactory.cpp
// Sergei <Neill3d> Solokhin

#include "SLiveLinkTechnocraneSourceFactory.h"

#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

#include "TechnocraneRuntimeSettings.h"

#define LOCTEXT_NAMESPACE "SLiveLinkTechnocraneSourceFactory"

void SLiveLinkTechnocraneSourceFactory::Construct(const FArguments& Args)
{
	OkClicked = Args._OnOkClicked;

	// default values

	const UTechnocraneRuntimeSettings* settings = GetDefault<UTechnocraneRuntimeSettings>();

	FIPv4Address address(FIPv4Address::Any);
	FIPv4Address::Parse(settings->NetworkServerAddressByDefault, address);

	FIPv4Endpoint Endpoint;
	Endpoint.Address = address;
	Endpoint.Port = settings->NetworkPortIdByDefault;

	const int serial_port = settings->SerialPortIdByDefault;
	m_SerialPortIndex = serial_port;

	auto GetNetworkVisibility = [&]()
	{
		TSharedPtr<SCheckBox> checkbox = m_UseNetworkConnection.Pin();
		if (checkbox.IsValid())
		{
			return checkbox->IsChecked() ? EVisibility::Visible : EVisibility::Collapsed;
		}
		return EVisibility::Collapsed;
	};

	auto GetSerialVisibility = [&]()
	{
		TSharedPtr<SCheckBox> checkbox = m_UseNetworkConnection.Pin();
		if (checkbox.IsValid())
		{
			return checkbox->IsChecked() ? EVisibility::Collapsed : EVisibility::Visible;
		}
		return EVisibility::Collapsed;
	};

	ChildSlot
		[
			SNew(SBox)
			.WidthOverride(250)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					.FillWidth(0.5f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("UseNetworkConnection", "Use Network Connection"))
					]
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Fill)
					.FillWidth(0.5f)
					[
						SAssignNew(m_UseNetworkConnection, SCheckBox)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					.FillWidth(0.5f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("NetworkBindAnyAddress", "Network Bind Any Address"))
						.Visibility_Lambda(GetNetworkVisibility)
					]
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Fill)
					.FillWidth(0.5f)
					[
						SAssignNew(m_NetworkBindAnyAddress, SCheckBox)
						.Visibility_Lambda(GetNetworkVisibility)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					.FillWidth(0.5f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("NetworkBroadcast", "Network Broadcast"))
						.Visibility_Lambda(GetNetworkVisibility)
					]
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Fill)
					.FillWidth(0.5f)
					[
						SAssignNew(m_NetworkBroadcast, SCheckBox)
						.Visibility_Lambda(GetNetworkVisibility)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					.FillWidth(0.5f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("UDPAddress", "Network Address:Port"))
						.Visibility_Lambda(GetNetworkVisibility)
					]
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Fill)
					.FillWidth(0.5f)
					[
						SAssignNew(m_NetworkAddress, SEditableTextBox)
						.Text(FText::FromString(Endpoint.ToString()))
						.OnTextCommitted(this, &SLiveLinkTechnocraneSourceFactory::OnEndpointChanged)
						.Visibility_Lambda(GetNetworkVisibility)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					.FillWidth(0.5f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("SerialPort", "Serial Port"))
						.Visibility_Lambda(GetSerialVisibility)
					]
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Fill)
					.FillWidth(0.5f)
					[
						SAssignNew(m_SerialPortBox, SNumericEntryBox<int>)
						.MinValue(0)
						.MaxValue(100)
						.Value(serial_port)
						.OnValueChanged(this, &SLiveLinkTechnocraneSourceFactory::NewSerialPortIndex)
						.Visibility_Lambda(GetSerialVisibility)
					]
				]
				+ SVerticalBox::Slot()
				.HAlign(HAlign_Right)
				.AutoHeight()
				[
					SNew(SButton)
					.OnClicked(this, &SLiveLinkTechnocraneSourceFactory::OnOkClicked)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Ok", "Ok"))
					]
				]
			]
		];

	// some default values
	const bool use_network = settings->bNetworkConnection;
	const bool broadcast = settings->bNetworkBroadcast;
	const bool bind_any_address = settings->bNetworkBindAnyAddress;

	{
		TSharedPtr<SCheckBox> checkbox = m_UseNetworkConnection.Pin();
		if (checkbox.IsValid())
		{
			return checkbox->SetIsChecked(use_network ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
		}
	}
	
	{
		TSharedPtr<SCheckBox> checkbox = m_NetworkBindAnyAddress.Pin();
		if (checkbox.IsValid())
		{
			return checkbox->SetIsChecked(bind_any_address ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
		}
	}
	
	{
		TSharedPtr<SCheckBox> checkbox = m_NetworkBroadcast.Pin();
		if (checkbox.IsValid())
		{
			return checkbox->SetIsChecked(broadcast ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
		}
	}
}

void SLiveLinkTechnocraneSourceFactory::OnEndpointChanged(const FText& NewValue, ETextCommit::Type)
{
	TSharedPtr<SEditableTextBox> address = m_NetworkAddress.Pin();
	if (address.IsValid())
	{
		FIPv4Endpoint Endpoint;
		if (!FIPv4Endpoint::Parse(NewValue.ToString(), Endpoint))
		{
			Endpoint.Address = FIPv4Address::Any;
			Endpoint.Port = 15246;
			address->SetText(FText::FromString(Endpoint.ToString()));
		}
	}
}

FReply SLiveLinkTechnocraneSourceFactory::OnOkClicked()
{
	TSharedPtr<SCheckBox> use_network = m_UseNetworkConnection.Pin();
	TSharedPtr<SCheckBox> bind_any_address = m_NetworkBindAnyAddress.Pin();
	TSharedPtr<SCheckBox> broadcast = m_NetworkBroadcast.Pin();
	TSharedPtr<SEditableTextBox> address = m_NetworkAddress.Pin();
	TSharedPtr<SNumericEntryBox<int>> serial_port = m_SerialPortBox.Pin();

	if (use_network.IsValid() && serial_port.IsValid() && address.IsValid()
		&& bind_any_address.IsValid() && broadcast.IsValid())
	{
		FIPv4Endpoint Endpoint;
		if (FIPv4Endpoint::Parse(address->GetText().ToString(), Endpoint))
		{
			SCreationInfo info{
				use_network->IsChecked(), 
				m_SerialPortIndex,
				Endpoint,
				bind_any_address->IsChecked(), 
				broadcast->IsChecked()
			};

			OkClicked.ExecuteIfBound(info);
		}
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
