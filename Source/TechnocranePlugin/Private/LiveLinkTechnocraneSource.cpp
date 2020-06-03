// Copyright (c) 2020 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// LiveLinkTechnocraneSource.cpp
// Sergei <Neill3d> Solokhin

#include "LiveLinkTechnocraneSource.h"

#include "ILiveLinkClient.h"
#include "LiveLinkTypes.h"

#include "Roles/LiveLinkCameraRole.h"
#include "Roles/LiveLinkCameraTypes.h"
#include "Roles/LiveLinkTransformRole.h"
#include "Roles/LiveLinkTransformTypes.h"

#include "Async/Async.h"
#include "HAL/RunnableThread.h"
#include "Json.h"

#include "TechnocraneRuntimeSettings.h"

#define LOCTEXT_NAMESPACE "TechnocraneLiveLinkSource"

////////////////////////////////////////////////////////////////////////////////////////////
// FLiveLinkTechnocraneSource

FLiveLinkTechnocraneSource::FLiveLinkTechnocraneSource(bool use_network, int serial_port, FIPv4Endpoint address, bool bind_any_address, bool broadcast)
	: m_Stopping(false)
	, m_CreateStaticSubject(true)
	, Thread(nullptr)
	, WaitTime(FTimespan::FromMilliseconds(100))
{
	// defaults
	m_UseNetwork = use_network;
	m_NetworkAddress = address;
	m_NetworkBindAnyAddress = bind_any_address;
	m_NetworkBroadcast = broadcast;
	m_SerialPort = serial_port;

	// Live link params
	m_SourceStatus = LOCTEXT("SourceStatus_Waiting", "Waiting");
	m_SourceType = LOCTEXT("TechnocraneLiveLinkSourceType", "Technocrane LiveLink");
	m_SourceMachineName = LOCTEXT("TechnocraneLiveLinkSourceMachineName", "localhost");

	
	m_Hardware = new NTechnocrane::CTechnocrane_Hardware();
	m_Hardware->Init(false, false, false);

	Start();
}

FLiveLinkTechnocraneSource::~FLiveLinkTechnocraneSource()
{
	Stop();
	if (Thread != nullptr)
	{
		Thread->WaitForCompletion();
		delete Thread;
		Thread = nullptr;
	}

	if (m_Hardware)
	{
		delete m_Hardware;
		m_Hardware = nullptr;
	}
}

void FLiveLinkTechnocraneSource::ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid)
{
	m_Client = InClient;
	m_SourceGuid = InSourceGuid;
}

bool FLiveLinkTechnocraneSource::IsSourceStillValid() const
{
	// Source is valid if we have a valid thread and socket
	bool bIsSourceValid = (!m_Stopping && Thread != nullptr);
	return bIsSourceValid;
}

bool FLiveLinkTechnocraneSource::RequestSourceShutdown()
{
	Stop();

	return true;
}

// FRunnable interface

void FLiveLinkTechnocraneSource::Start()
{
	ThreadName = "Technocrane UDP Receiver ";
	ThreadName.AppendInt(FAsyncThreadIndex::GetNext());

	Thread = FRunnableThread::Create(this, *ThreadName, 128 * 1024, TPri_AboveNormal, FPlatformAffinity::GetPoolThreadMask());
}

void FLiveLinkTechnocraneSource::Stop()
{
	m_Stopping = true;
}

uint32 FLiveLinkTechnocraneSource::Run()
{
	bool first_enter{ true };

	while (!m_Stopping)
	{
	
		if (!m_Hardware)
			break;

		KeepLive(first_enter);
		first_enter = false;

		if (m_Hardware->IsReady())
		{
			const bool packed_data = GetDefault<UTechnocraneRuntimeSettings>()->bPacketContainsRawAndCalibratedData;

			size_t index = 0;
			NTechnocrane::STechnocrane_Packet	packet;

			if (m_Hardware->FetchDataPacket(packet, 1, index, packed_data) > 0)
			{
				m_SourceStatus = LOCTEXT("SourceStatus_Receiving", "Receiving");
				AsyncTask(ENamedThreads::GameThread, [this, packet]() { HandleReceivedData(packet); });
			}
		}
	}

	if (m_Hardware->IsReady())
	{
		m_Hardware->StopDataStream();
		m_Hardware->Close();
	}

	return 0;
}

void FLiveLinkTechnocraneSource::HandleReceivedData(const NTechnocrane::STechnocrane_Packet& packet)
{
	if (m_Stopping)
		return;

	if (!GetDefault<UTechnocraneRuntimeSettings>())
		return;

	const float x = packet.Position[2];
	const float y = packet.Position[0];
	const float z = packet.Position[1];

	const float space_scale = GetDefault<UTechnocraneRuntimeSettings>()->SpaceScaleByDefault;

	FVector		v(space_scale * y, space_scale * x, space_scale * z);
	FRotator	rot(packet.Tilt, 90.0f + packet.Pan, packet.Roll);

	const float TrackPosition = space_scale * packet.TrackPos;

	float zoom = 1.0;
	bool IsZoomCalibrated = NTechnocrane::ComputeZoomf(zoom, packet.Zoom, 0.0f, 100.0f);
	
	bool IsIrisCalibrated = false;

	float iris = 1.0;
	const bool packed_data = false;
	if (!packed_data)
	{
		
		IsIrisCalibrated = NTechnocrane::ComputeIrisf(iris, packet.Iris, 0.0f, 100.0f);
	}

	float focus = 1.0;
	bool IsFocusCalibrated = NTechnocrane::ComputeFocusf(focus, packet.Focus, 0.0f, 100.0f);

	//
	// static data

	FName subject_name("CameraSubject");

	if (m_CreateStaticSubject)
	{
		FLiveLinkStaticDataStruct StaticDataStruct = FLiveLinkStaticDataStruct(FLiveLinkCameraStaticData::StaticStruct());
		FLiveLinkCameraStaticData& StaticData = *StaticDataStruct.Cast<FLiveLinkCameraStaticData>();

		StaticData.bIsFieldOfViewSupported = false;
		StaticData.bIsFocalLengthSupported = true;
		StaticData.bIsFocusDistanceSupported = true;
		StaticData.bIsApertureSupported = true;
		
		m_Client->PushSubjectStaticData_AnyThread({ m_SourceGuid, subject_name }, ULiveLinkCameraRole::StaticClass(), MoveTemp(StaticDataStruct));

		m_CreateStaticSubject = false;
	}


	//
	// dynamic data

	FLiveLinkFrameDataStruct FrameDataStruct = FLiveLinkFrameDataStruct(FLiveLinkCameraFrameData::StaticStruct());
	FLiveLinkCameraFrameData& FrameData = *FrameDataStruct.Cast<FLiveLinkCameraFrameData>();

	FrameData.FocalLength = zoom;
	FrameData.FocusDistance = space_scale * focus;
	FrameData.Aperture = iris;

	FVector RawPosition = v;
	FVector RawRotation = FVector(packet.Pan, packet.Tilt, packet.Roll);

	bool HasTimeCode = packet.HasTimeCode();
	int PacketNumber = packet.PacketNumber;

	FTimecode TimeCode;
	FFrameRate FrameRate(GetDefault<UTechnocraneRuntimeSettings>()->CameraFrameRate);

	if (packet.HasTimeCode())
	{
		TimeCode.Hours = packet.hours;
		TimeCode.Minutes = packet.minutes;
		TimeCode.Seconds = packet.seconds;
		TimeCode.Frames = packet.frames;
	}
	else
	{
		TimeCode.FromFrameNumber(FFrameNumber(static_cast<int32>(packet.frames)),
			FrameRate, false);
	}

	FrameData.Transform.SetTranslation(RawPosition);
	FrameData.Transform.SetRotation(rot.Quaternion());
	FrameData.MetaData.SceneTime.Time = TimeCode.ToFrameNumber(FrameRate);
	
	FrameData.MetaData.StringMetaData.Add("CameraOn", (packet.CameraOn) ? "1" : "0");
	FrameData.MetaData.StringMetaData.Add("Running", (packet.Running) ? "1" : "0");

	FrameData.MetaData.StringMetaData.Add("IsZoomCalibrated", (IsZoomCalibrated) ? "1" : "0");
	FrameData.MetaData.StringMetaData.Add("IsFocusCalibrated", (IsFocusCalibrated) ? "1" : "0");
	FrameData.MetaData.StringMetaData.Add("IsIrisCalibrated", (IsIrisCalibrated) ? "1" : "0");

	FrameData.MetaData.StringMetaData.Add("RawTimeCode", TimeCode.ToString());
	
	FrameData.WorldTime = FPlatformTime::Seconds();
	m_Client->PushSubjectFrameData_AnyThread({ m_SourceGuid, subject_name }, MoveTemp(FrameDataStruct));
}

bool FLiveLinkTechnocraneSource::KeepLive(const bool compare_options)
{
	bool need_restart = false;

	if (m_Hardware->IsReady() && compare_options)
	{
		NTechnocrane::SOptions	options;
		options = m_Hardware->GetOptions();

		NTechnocrane::SOptions camera_options(options);
		PrepareOptions(camera_options);

		if (!CompareOptions(options, camera_options))
		{
			need_restart = true;
		}
	}

	if (need_restart)
	{
		m_Hardware->StopDataStream();
		m_Hardware->Close();
	}

	if (!m_Hardware->IsReady() )
	{
		m_SourceStatus = LOCTEXT("SourceStatus_Waiting", "Waiting");
		m_CooldownTimer -= 1;

		if (m_CooldownTimer <= 0)
		{
			NTechnocrane::SOptions	options;
			PrepareOptions(options);
			m_Hardware->ClearLastError();

			if (m_Hardware->Open(options))
			{
				m_Hardware->StartDataStream();
			}
			else
			{
				m_CooldownTimer = 10000;

				UE_LOG(LogTemp, Log, TEXT("Failed to connect to a hardware on a specified port"));
				m_SourceStatus = LOCTEXT("SourceStatus_Failed", "Failed to Connect");
				return false;
			}
		}
	}
	return true;
}

void FLiveLinkTechnocraneSource::PrepareOptions(NTechnocrane::SOptions& options)
{
	options.m_UseNetworkConnection = m_UseNetwork;

	options.m_BindAnyAddress = m_NetworkBindAnyAddress;
	options.m_Broadcast = m_NetworkBroadcast;
	options.m_NetworkPort = m_NetworkAddress.Port; // server port

	options.m_SerialPort = m_SerialPort;
}

bool FLiveLinkTechnocraneSource::CompareOptions(const NTechnocrane::SOptions& a, const NTechnocrane::SOptions& b)
{
	if (a.m_UseNetworkConnection != b.m_UseNetworkConnection)
	{
		return false;
	}

	if (a.m_NetworkPort != b.m_NetworkPort)
	{
		return false;
	}

	if (a.m_SerialPort != b.m_SerialPort)
	{
		return false;
	}

	return true;
}

#undef LOCTEXT_NAMESPACE