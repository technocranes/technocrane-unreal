// Copyright (c) 2018 Technocrane s.r.o. 
//
// TechnocraneCamera.cpp
// Sergei <Neill3d> Solokhin 2018

#include "TechnocranePrivatePCH.h"
#include <Public/TechnocraneCamera.h>
#include <Public/ITechnocranePlugin.h>
#include <Runtime/CinematicCamera/Public/CineCameraComponent.h>

#include <../ThirdParty/TechnocraneSDK/include/technocrane_hardware.h>

// Sets default values
ATechnocraneCamera::ATechnocraneCamera(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	if (ITechnocranePlugin::IsAvailable())
	{
		mHardware = new NTechnocrane::CTechnocrane_Hardware();
		mHardware->Init();
	}
	
	//
	if (UCineCameraComponent* comp = GetCineCameraComponent())
	{
		comp->FocusSettings.FocusMethod = ECameraFocusMethod::None;
		comp->SetFilmbackPresetByName("35mm Full Aperture");
	}

	//
	Port = 1;
	Live = false;
	SpaceScale = 100.0f;
	TrackPosition = 0.0f;

	SetActorScale3D(FVector(0.5f, 0.5f, 0.5f));

	//
	ZoomRange = FVector2D(0.0f, 100.0f);
	IrisRange = FVector2D(0.0f, 100.0f);
	FocusRange = FVector2D(0.0f, 100.0f);
}
ATechnocraneCamera::~ATechnocraneCamera()
{
	if (nullptr != mHardware)
	{
		delete mHardware;
		mHardware = nullptr;
	}
}

// Called when the game starts or when spawned
void ATechnocraneCamera::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATechnocraneCamera::Tick(float DeltaTime)
{
	if (Live && mHardware)
	{
		size_t index = 0;
		NTechnocrane::STechnocrane_Packet	packet;

		if (mHardware->FetchDataPacket(packet, 1, index) > 0)
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

			SetActorLocation(v);
			SetActorRotation(rot);

			if (UCineCameraComponent* comp = GetCineCameraComponent())
			{
				double zoom = 1.0;
				NTechnocrane::ComputeZoom(zoom, packet.Zoom, ZoomRange.X, ZoomRange.Y);
				comp->CurrentFocalLength = zoom;

				double iris = 1.0;
				NTechnocrane::ComputeIris(iris, packet.Iris, IrisRange.X, IrisRange.Y);
				comp->CurrentAperture = iris;

				double focus = 1.0;
				NTechnocrane::ComputeFocus(focus, packet.Focus, FocusRange.X, FocusRange.Y);
				comp->FocusSettings.ManualFocusDistance = focus;
			}
		}
		
	}

	Super::Tick(DeltaTime);
}

#if WITH_EDITOR
void ATechnocraneCamera::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	SwitchLive();
}

void ATechnocraneCamera::PostEditUndo()
{
	Super::PostEditUndo();

	SwitchLive();
}

#endif

bool ATechnocraneCamera::SwitchLive()
{
	if (!mHardware)
		return false;

	if (Live)
	{
		const int32 port_index = Port;

		NTechnocrane::SOptions	options;
		options = mHardware->GetOptions();

		if (mHardware->IsReady() && (options.port != port_index))
		{
			mHardware->StopDataStream();
			mHardware->Close();
		}

		if (false == mHardware->IsReady())
		{
			// COM Nnn	
			options.port = port_index;
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

	return true;
}