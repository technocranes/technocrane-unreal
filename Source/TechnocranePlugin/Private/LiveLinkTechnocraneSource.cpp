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
#include "LiveLinkTechnocraneTypes.h"

#include "Windows/WindowsPlatformTime.h"

#define LOCTEXT_NAMESPACE "TechnocraneLiveLinkSource"

////////////////////////////////////////////////////////////////////////////////////////////
// FLiveLinkTechnocraneSource

FLiveLinkTechnocraneSource::FLiveLinkTechnocraneSource(bool use_network, int serial_port, FIPv4Endpoint address, bool bind_any_address, bool broadcast)
	: m_Stopping(false)
	, m_CreateStaticSubject(true)
	, m_Thread(nullptr)
{
	// defaults
	m_UseNetwork = use_network;
	m_NetworkAddress = address;
	m_NetworkBindAnyAddress = bind_any_address;
	m_NetworkBroadcast = broadcast;
	m_SerialPort = serial_port;

	// Live link params
	m_SourceStatus = LOCTEXT("SourceStatus_Waiting", "Waiting");
	m_SourceType = LOCTEXT("TechnocraneLiveLinkSourceType", "Technocrane");
	
	if (use_network)
	{
		m_SourceMachineName = address.ToText();
	}
	else
	{
		m_SourceMachineName = FText::FromString(TEXT("COM ") + FString::FromInt(serial_port));
	}
	
	m_Hardware = new NTechnocrane::CTechnocrane_Hardware();
	m_Hardware->Init(false, false, false);

	Start();
}

FLiveLinkTechnocraneSource::~FLiveLinkTechnocraneSource()
{
	Stop();
	if (m_Thread != nullptr)
	{
		m_Thread->WaitForCompletion();
		delete m_Thread;
		m_Thread = nullptr;
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
	bool is_source_valid = (!m_Stopping && m_Thread != nullptr && m_Hardware != nullptr);
	return is_source_valid;
}

bool FLiveLinkTechnocraneSource::RequestSourceShutdown()
{
	Stop();
	return true;
}

// FRunnable interface

void FLiveLinkTechnocraneSource::Start()
{
	m_ThreadName = "Technocrane Receiver ";
	m_ThreadName.AppendInt(FAsyncThreadIndex::GetNext());

	m_Thread = FRunnableThread::Create(this, *m_ThreadName, 128 * 1024, TPri_AboveNormal, FPlatformAffinity::GetPoolThreadMask());
}

void FLiveLinkTechnocraneSource::Stop()
{
	m_Stopping = true;
}

void FLiveLinkTechnocraneSource::UpdateStatus(const NTechnocrane::STechnocrane_Packet& packet, const bool force_update)
{
	const bool flags[4] = { packet.HasTimeCode(), packet.IsZoomCalibrated, packet.IsIrisCalibrated, packet.IsFocusCalibrated };

	if (force_update || flags[0] != m_LastStatusFlags[0] || flags[1] != m_LastStatusFlags[1] || flags[2] != m_LastStatusFlags[2] || flags[3] != m_LastStatusFlags[3])
	{
		FString text;
		text = FString::Format(TEXT("Receiving [T:{0}, Z:{1}, F:{2}, I:{3}]"),
			{
				(packet.HasTimeCode()) ? TEXT("Y") : TEXT("N"),
				(packet.IsZoomCalibrated) ? TEXT("Y") : TEXT("N"),
				(packet.IsFocusCalibrated) ? TEXT("Y") : TEXT("N"),
				(packet.IsIrisCalibrated) ? TEXT("Y") : TEXT("N")
			}
		);

		m_SourceStatus = FText::FromString(text);

		memcpy(m_LastStatusFlags, flags, sizeof(bool) * 4);
	}
}

uint32 FLiveLinkTechnocraneSource::Run()
{
	bool first_enter{ true };

	while (!m_Stopping)
	{
		if (!m_Hardware)
			break;

		KeepLive(first_enter);
		
		if (m_Hardware->IsReady())
		{
			const bool packed_data = GetDefault<UTechnocraneRuntimeSettings>()->bPacketContainsRawAndCalibratedData;

			size_t index = 0;
			NTechnocrane::STechnocrane_Packet	packet;

			if (m_Hardware->FetchDataPacket(packet, 1, index, packed_data) > 0)
			{
				UpdateStatus(packet, first_enter);
				AsyncTask(ENamedThreads::GameThread, [this, packet]() { HandleReceivedData(packet); });
			}
		}

		first_enter = false;
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

	const UTechnocraneRuntimeSettings* settings = GetDefault<UTechnocraneRuntimeSettings>();

	if (!settings)
		return;

	const float x = packet.Position[2];
	const float y = packet.Position[0];
	const float z = packet.Position[1];

	const float space_scale = settings->SpaceScaleByDefault;

	FVector		v(space_scale * y, space_scale * x, space_scale * z);
	FRotator	rot(packet.Tilt, 90.0f + packet.Pan, packet.Roll);

	const float track_position = space_scale * packet.TrackPos;

	float zoom = 1.0;
	bool IsZoomCalibrated = NTechnocrane::ComputeZoomf(zoom, packet.Zoom, settings->ZoomRange.Min, settings->ZoomRange.Max);
	
	float iris = 1.0;
	bool IsIrisCalibrated = false;
	
	const bool packed_data = settings->bPacketContainsRawAndCalibratedData;
	if (!packed_data)
	{
		IsIrisCalibrated = NTechnocrane::ComputeIrisf(iris, packet.Iris, settings->IrisRange.Min, settings->IrisRange.Max);
	}

	float focus = 1.0;
	bool IsFocusCalibrated = NTechnocrane::ComputeFocusf(focus, packet.Focus, settings->FocusRange.Min, settings->FocusRange.Max);

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
		
		StaticData.PropertyNames.Reset(static_cast<int32>(EPacketProperties::Total));

		const char* property_names[static_cast<int32>(EPacketProperties::Total)] = {
			"TrackPosition",
			"PacketNumber",
			"X",
			"Y",
			"Z",
			"Pan",
			"Tilt",
			"Roll",
			"CameraOn",
			"Running"
		};

		for (const char* name : property_names)
		{
			StaticData.PropertyNames.Add(name);
		}

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

	const bool has_timecode = packet.HasTimeCode();
	const int32 packet_number = packet.PacketNumber;

	FTimecode TimeCode;
	FFrameRate FrameRate(settings->CameraFrameRate);

	if (has_timecode)
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

	FrameData.Transform.SetTranslation(v);
	FrameData.Transform.SetRotation(rot.Quaternion());
	FrameData.MetaData.SceneTime.Time = TimeCode.ToFrameNumber(FrameRate);
	
	FrameData.MetaData.StringMetaData.Add("CameraOn", (packet.CameraOn) ? "1" : "0");
	FrameData.MetaData.StringMetaData.Add("Running", (packet.Running) ? "1" : "0");

	FrameData.MetaData.StringMetaData.Add("IsZoomCalibrated", (IsZoomCalibrated) ? "1" : "0");
	FrameData.MetaData.StringMetaData.Add("IsFocusCalibrated", (IsFocusCalibrated) ? "1" : "0");
	FrameData.MetaData.StringMetaData.Add("IsIrisCalibrated", (IsIrisCalibrated) ? "1" : "0");
	
	FrameData.MetaData.StringMetaData.Add("PacketNumber", FString::FromInt(packet_number));

	FrameData.MetaData.StringMetaData.Add("HasTimeCode", (has_timecode) ? "1" : "0");
	FrameData.MetaData.StringMetaData.Add("RawTimeCode", TimeCode.ToString());
	
	FrameData.MetaData.StringMetaData.Add("FrameRate", FrameRate.ToPrettyText().ToString());

	const float property_values[static_cast<int32>(EPacketProperties::Total)] = {
		track_position,
		static_cast<float>(packet_number),
		packet.Position[0],
		packet.Position[1],
		packet.Position[2],
		packet.Pan,
		packet.Tilt,
		packet.Roll,
		static_cast<float>(packet.CameraOn),
		static_cast<float>(packet.Running)
	};

	FrameData.PropertyValues.Reserve(static_cast<int32>(EPacketProperties::Total));
	for (const float prop : property_values)
	{
		FrameData.PropertyValues.Add(prop);
	}

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
		
		const double curr_time{ FPlatformTime::Seconds() };
		constexpr double eps_time{ 7.0 };

		if (curr_time - m_CooldownTimer > eps_time)
		{
			m_CooldownTimer = curr_time;

			NTechnocrane::SOptions	options;
			PrepareOptions(options);
			m_Hardware->ClearLastError();

			if (m_Hardware->Open(options))
			{
				m_Hardware->StartDataStream();
			}
			else
			{
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