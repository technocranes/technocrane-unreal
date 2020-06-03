// Copyright (c) 2020 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// SLiveLinkTechnocraneSourceFactory.h
// Sergei <Neill3d> Solokhin

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Widgets/SCompoundWidget.h"
#include "Input/Reply.h"
#include "Types/SlateEnums.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"

class SEditableTextBox;
class SCheckBox;
class STextBlock;

struct SCreationInfo
{
	bool	m_UseNetwork;
	int32	m_SerialPort;
	FIPv4Endpoint	m_Address;
	bool		m_NetworkBindAny;
	bool		m_NetworkBroadcast;
};

class SLiveLinkTechnocraneSourceFactory : public SCompoundWidget
{
public:
	// use network, serial port, address, bind any, broadcast
	DECLARE_DELEGATE_OneParam(FOnOkClicked, SCreationInfo);

	SLATE_BEGIN_ARGS(SLiveLinkTechnocraneSourceFactory) {}
		SLATE_EVENT(FOnOkClicked, OnOkClicked)
	SLATE_END_ARGS()

	void Construct(const FArguments& Args);

private:
	void OnEndpointChanged(const FText& NewValue, ETextCommit::Type);

	FReply OnOkClicked();

	TWeakPtr<SCheckBox>			m_UseNetworkConnection;
	TWeakPtr<SCheckBox>			m_NetworkBindAnyAddress;
	TWeakPtr<SCheckBox>			m_NetworkBroadcast;
	
	TWeakPtr<SEditableTextBox>		m_NetworkAddress;
	TWeakPtr<SNumericEntryBox<int>>	m_SerialPortBox;

	int32 m_SerialPortIndex{ 1 };
	
	FOnOkClicked OkClicked;

	void NewSerialPortIndex(int new_value) { m_SerialPortIndex = new_value; }
};