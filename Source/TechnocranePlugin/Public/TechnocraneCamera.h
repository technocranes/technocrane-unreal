// Copyright (c) 2019 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// TechnocraneCamera.h
// Sergei <Neill3d> Solokhin 2019

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <Runtime/CinematicCamera/Public/CineCameraActor.h>
#include "Misc/Timecode.h"

#include <technocrane_hardware.h>

#include "TechnocraneCamera.generated.h"


UCLASS(Blueprintable, ClassGroup = Technocrane, Category = "Technocrane", meta = (BlueprintSpawnableComponent))
class TECHNOCRANEPLUGIN_API ATechnocraneCamera : public ACineCameraActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATechnocraneCamera(const FObjectInitializer& ObjectInitializer);
	virtual ~ATechnocraneCamera();

	const bool IsReady() const;

	const int GetLastError() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Connection)
		bool UseNetworkConnection;

	/** Controls the amount of rails for the crane rig. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Connection, meta = (ClampMin = 0, ClampMax = 10))
		int SerialPort;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Connection)
		bool NetworkBindAnyAddress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Connection)
		FString NetworkAddress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Connection)
		int NetworkPort;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = Connection)
		bool Live;

	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Raw Data")
		float TrackPosition;
	
	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Raw Data")
		FVector RawPosition;

	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Raw Data")
		FVector RawRotation;

	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Raw Data")
		bool HasTimeCode;

	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Raw Data")
		FTimecode	TimeCode;
	
	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Raw Data")
		int32 PacketNumber;

	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Raw Data")
		bool IsZoomCalibrated;

	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Raw Data")
		bool IsIrisCalibrated;

	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Raw Data")
		bool IsFocusCalibrated;

	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Raw Data")
		float RawZoom;

	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Raw Data")
		float RawIris;

	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Raw Data")
		float RawFocus;

	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Raw Data")
		float CalibratedZoom;

	UPROPERTY(VisibleAnywhere, SkipSerialization, BlueprintReadOnly, Category = "Tracking Raw Data")
		float CalibratedFocus;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Tracking Options")
		float SpaceScale;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Tracking Options")
		FFrameRate FrameRate;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Tracking Options")
		FVector2D ZoomRange;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Tracking Options")
		FVector2D IrisRange;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Tracking Options")
		FVector2D FocusRange;

	/**
	 * Gets the playback Timecode of the level sequence
	 * @return the current playback Timecode
	 */
	UFUNCTION(BlueprintPure, Category = "Tracking Raw Data")
		FTimecode GetPlaybackTimecode();

#if WITH_EDITOR
	virtual bool CanEditChange(const UProperty* InProperty) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditUndo() override;
#endif

protected:

	bool m_IsInitialized;

	bool SwitchLive();

#if defined(TECHNOCRANESDK)
	NTechnocrane::CTechnocrane_Hardware*	mHardware;
	void PrepareOptions(NTechnocrane::SOptions& options);
	bool CompareOptions(const NTechnocrane::SOptions& a, const NTechnocrane::SOptions& b);
#endif
};
