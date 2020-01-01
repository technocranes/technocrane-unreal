// Copyright (c) 2019 Technocrane s.r.o. 
//
// TechnocraneCamera.cpp
// Sergei <Neill3d> Solokhin 2019

#include "TechnocraneCamera.h"
#include "TechnocranePrivatePCH.h"
#include "TechnocraneRuntimeSettings.h"
#include "ITechnocranePlugin.h"
#include <Runtime/CinematicCamera/Public/CineCameraComponent.h>

//#include <ThirdParty/TechnocraneSDK/include/technocrane_hardware.h>

// Sets default values
ATechnocraneCamera::ATechnocraneCamera(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, FrameRate(25, 1)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	//FPlatformProcess::GetDllHandle("TechnocraneLib.dll");

	if (ITechnocranePlugin::IsAvailable())
	{
#if defined(TECHNOCRANESDK)
		mHardware = new NTechnocrane::CTechnocrane_Hardware();
		mHardware->Init();
#endif
	}
	
	//
	if (UCineCameraComponent* comp = GetCineCameraComponent())
	{
		comp->FocusSettings.FocusMethod = ECameraFocusMethod::None;
		comp->SetFilmbackPresetByName("35mm Full Aperture");
	}
	
	//
	if (const UTechnocraneRuntimeSettings* settings = GetDefault<UTechnocraneRuntimeSettings>())
	{
		UseNetworkConnection = settings->bNetworkConnection;
		SerialPort = settings->SerialPortIdByDefault;
		NetworkBindAnyAddress = settings->bNetworkBindAnyAddress;
		NetworkAddress = settings->NetworkServerAddressByDefault;
		NetworkPort = settings->NetworkPortIdByDefault;

		Live = settings->bLiveByDefault;
	}
	else
	{
		UseNetworkConnection = false;
		SerialPort = 1;
		NetworkBindAnyAddress = true;
		NetworkAddress = "127.0.0.1";
		NetworkPort = 15245;
		Live = false;
	}
	
	SpaceScale = GetDefault<UTechnocraneRuntimeSettings>()->SpaceScaleByDefault;
	//FrameRate = 25.0f;
	TrackPosition = 0.0f;

	//SetActorScale3D(FVector(0.5f, 0.5f, 0.5f));
	
	//
	ZoomRange = FVector2D(0.0f, 100.0f);
	IrisRange = FVector2D(0.0f, 100.0f);
	FocusRange = FVector2D(0.0f, 100.0f);

	m_IsInitialized = false;
}
ATechnocraneCamera::~ATechnocraneCamera()
{
#if defined(TECHNOCRANESDK)
	if (nullptr != mHardware)
	{
		delete mHardware;
		mHardware = nullptr;
	}
#endif
}

// Called when the game starts or when spawned
void ATechnocraneCamera::BeginPlay()
{
	Super::BeginPlay();
	
}

const bool ATechnocraneCamera::IsReady() const {
#if defined(TECHNOCRANESDK)
	return mHardware->IsReady();
#else
	return false;
#endif
}

const int ATechnocraneCamera::GetLastError() const {
#if defined(TECHNOCRANESDK)
	return mHardware->GetLastError();
#else
	return 0;
#endif
}

FTimecode ATechnocraneCamera::GetPlaybackTimecode()
{
	return TimeCode;
}

// Called every frame
void ATechnocraneCamera::Tick(float DeltaTime)
{
#if defined(TECHNOCRANESDK)
	if (Live && mHardware)
	{
		if (m_IsInitialized == false)
		{
			SwitchLive();
			m_IsInitialized = true;
		}

		if (!mHardware->IsReady())
		{
			Live = false;
			mHardware->StopDataStream();
			mHardware->Close();
			Super::Tick(DeltaTime);
			return;
		}

		const bool packed_data = GetDefault<UTechnocraneRuntimeSettings>()->bPacketContainsRawAndCalibratedData;

		size_t index = 0;
		NTechnocrane::STechnocrane_Packet	packet;

		if (mHardware->FetchDataPacket(packet, 1, index, packed_data) > 0)
		{
			FVector		v(SpaceScale * packet.Position[0],
							SpaceScale * packet.Position[1],
							SpaceScale * packet.Position[2]);
			FRotator	rot(packet.Tilt, 90.0f + packet.Pan, packet.Roll);

			TrackPosition = SpaceScale * packet.TrackPos;

			RawPosition = v;
			RawRotation = FVector(packet.Pan, packet.Tilt, packet.Roll);

			HasTimeCode = packet.HasTimeCode();
			PacketNumber = packet.PacketNumber;

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

			if (UCineCameraComponent* comp = GetCineCameraComponent())
			{
				comp->SetRelativeLocationAndRotation(v, rot);

				// print out raw values
				RawFocus = packet.Focus;
				RawZoom = packet.Zoom;
				RawIris = packet.Iris;

				// compute final values

				float zoom = 1.0;
				IsZoomCalibrated = NTechnocrane::ComputeZoom(zoom, packet.Zoom, ZoomRange.X, ZoomRange.Y);
				comp->CurrentFocalLength = zoom;
				CalibratedZoom = zoom;

				if (!packed_data)
				{
					float iris = 1.0;
					IsIrisCalibrated = NTechnocrane::ComputeIris(iris, packet.Iris, IrisRange.X, IrisRange.Y);
					comp->CurrentAperture = iris;
				}
				
				float focus = 1.0;
				IsFocusCalibrated = NTechnocrane::ComputeFocus(focus, packet.Focus, FocusRange.X, FocusRange.Y);
				comp->FocusSettings.ManualFocusDistance = SpaceScale * focus;

				CalibratedFocus = SpaceScale * focus;
			}
		}
		
	}
#endif
	Super::Tick(DeltaTime);
}

#if WITH_EDITOR
bool ATechnocraneCamera::CanEditChange(const UProperty* InProperty) const
{
	const bool ParentVal = Super::CanEditChange(InProperty);

	//show the projectile type dropdown only if the delivery method is projecile
	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ATechnocraneCamera, SerialPort))
	{
		return ParentVal && (Live == false) && (UseNetworkConnection == false);
	}
	else 
	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ATechnocraneCamera, UseNetworkConnection)
		|| InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ATechnocraneCamera, NetworkBindAnyAddress)
		|| InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ATechnocraneCamera, NetworkPort))
	{
		return ParentVal && (Live == false);
	}
	/*Add any "else if" chains here.*/
	else if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ATechnocraneCamera, NetworkAddress))
	{
		return ParentVal && (Live == false) && (NetworkBindAnyAddress == false);
	}
	
	return ParentVal;
}

void ATechnocraneCamera::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ATechnocraneCamera, Live))
	{
		SwitchLive();
	}
}

void ATechnocraneCamera::PostEditUndo()
{
	Super::PostEditUndo();

	SwitchLive();
}

#endif

bool ATechnocraneCamera::SwitchLive()
{
#if defined(TECHNOCRANESDK)
	if (!mHardware)
		return false;

	if (Live)
	{
		NTechnocrane::SOptions	options;
		options = mHardware->GetOptions();

		bool need_restart = false;

		if (mHardware->IsReady())
		{
			NTechnocrane::SOptions camera_options(options);
			PrepareOptions(camera_options);

			if (!CompareOptions(options, camera_options))
			{
				need_restart = true;
			}
		}

		if (need_restart)
		{
			mHardware->StopDataStream();
			mHardware->Close();
		}

		if (false == mHardware->IsReady())
		{
			PrepareOptions(options);
			mHardware->ClearLastError();

			if (true == mHardware->Open(options))
			{
				mHardware->StartDataStream();
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("Failed to connect to a hardware on a specified port"));
				Live = false;
			}
		}
	}
	else
	{
		if (mHardware->IsReady())
		{
			mHardware->StopDataStream();
			mHardware->Close();
		}
	}
#endif
	return true;
}

#if defined(TECHNOCRANESDK)
void ATechnocraneCamera::PrepareOptions(NTechnocrane::SOptions& options)
{
	options.m_UseNetworkConnection = UseNetworkConnection;

	options.m_BindAnyAddress = NetworkBindAnyAddress;
	//options.m_NetworkAddress = htonl(INADDR_ANY);
	options.m_NetworkPort = NetworkPort; // server port

	options.m_SerialPort = SerialPort;
	//options.m_BaudRate = 115200;
}

bool ATechnocraneCamera::CompareOptions(const NTechnocrane::SOptions& a, const NTechnocrane::SOptions& b)
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

#endif