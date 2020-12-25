// Copyright (c) 2020 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// LiveLinkTechnocraneSource.h
// Sergei <Neill3d> Solokhin

#pragma once

#include "ILiveLinkSource.h"
#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"
#include "IMessageContext.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"

#include <technocrane_hardware.h>

class FRunnableThread;
class FSocket;
class ILiveLinkClient;
class ISocketSubsystem;

class TECHNOCRANEPLUGIN_API FLiveLinkTechnocraneSource : public ILiveLinkSource, public FRunnable
{
public:
	//! a constructor
	FLiveLinkTechnocraneSource(bool use_network, int serial_port, FIPv4Endpoint endpoint, bool bind_any_address, bool broadcast);
	//! a destructor
	virtual ~FLiveLinkTechnocraneSource();

	// ILiveLinkSource interface
	
	void ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid) override;
	
	bool IsSourceStillValid() const override;
	bool RequestSourceShutdown() override;

	FText GetSourceType() const override { return m_SourceType; }
	FText GetSourceMachineName() const override { return m_SourceMachineName; }
	FText GetSourceStatus() const override { return m_SourceStatus; }

	// Begin FRunnable Interface

	virtual bool Init() override { return true; }
	virtual uint32 Run() override;
	void Start();
	virtual void Stop() override;
	virtual void Exit() override { }

	// End FRunnable Interface

	void HandleReceivedData(const NTechnocrane::STechnocrane_Packet& packet);

private:
	
	ILiveLinkClient*		m_Client{ nullptr };

	// Our identifier in LiveLink
	FGuid					m_SourceGuid;

	FText					m_SourceType;
	FText					m_SourceMachineName;
	FText					m_SourceStatus;

	bool					m_LastStatusFlags[4]{ false };

	bool					m_UseNetwork;
	FIPv4Endpoint			m_NetworkAddress;
	bool					m_NetworkBindAnyAddress;
	bool					m_NetworkBroadcast;

	int32					m_SerialPort;

	
	// Threadsafe Bool for terminating the main thread loop
	FThreadSafeBool			m_Stopping;
	FThreadSafeBool			m_CreateStaticSubject;

	// Thread to run socket operations on
	FRunnableThread*		m_Thread;

	// Name of the sockets thread
	FString					m_ThreadName;


	double					m_CooldownTimer{ 0.0 };

	NTechnocrane::CTechnocrane_Hardware*	m_Hardware{ nullptr };

	void PrepareOptions(NTechnocrane::SOptions& options);
	bool CompareOptions(const NTechnocrane::SOptions& a, const NTechnocrane::SOptions& b);

	bool KeepLive(const bool compare_options=false);
	void UpdateStatus(const NTechnocrane::STechnocrane_Packet& packet, const bool force_update);
};