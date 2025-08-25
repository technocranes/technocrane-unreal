// Copyright (c) 2025 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// TechnocraneData.h
// Sergei <Neill3d> Solokhin

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TechnocraneData.generated.h"

/** Calculated values from simulation */
USTRUCT(BlueprintType)
struct FCraneSimulationData
{
	GENERATED_USTRUCT_BODY()

	/** Height from the ground to camera in cm */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Output", meta = (Units = cm))
	float GroundHeight{ 0.0f };

	/** the angle for beans being tilted up or down, in degrees */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Output", meta = (Units = degrees))
	float TiltAngle{ 0.0f };

	/** the current length of beans, in cm */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Output", meta = (Units = cm))
	float ExtensionLength{ 0.0f };
};

/** Structure that defines a level up table entry */
USTRUCT(BlueprintType)
struct FCraneData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:

	FCraneData()
		: Name("TechnoDolly")
		, ZOffsetOnGround(36.0f)
		, ZOffsetOnTracks(20.0f)
		, TracksSupport(0)
		, BeamsCount(3)
		, ColumnCount(0)
		, TiltMin(55.0f)
		, TiltMax(55.0f)
		, PanMin(270.0f)
		, PanMax(270.0f)
		, CameraOffsetX(26.0f)
		, CraneModelPath("/TechnocranePlugin/TechnodollyModel")
	{}

	
	/** Name of a crane preset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Data)
		FString Name;

	/** offset from a ground */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Data)
		float ZOffsetOnGround;

	/** offset from a tracks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tracks)
		float ZOffsetOnTracks;

	/** are tracks supported for the crane preset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tracks)
		int32 TracksSupport;

	/** number of beams in the crane */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Data)
		int32 BeamsCount;

	/** number of columns in the crane */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Data)
		int32 ColumnCount;

	/** use a specific bone to peform a horizontal rotation (column around up axis) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Data)
		FName ColumnRotationBone;

	/** Minimum angle to tilt */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Limits)
		float TiltMin;

	/** Maximum angle to tilt */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Limits)
		float TiltMax;

	/** Minimum angle to pan */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Limits)
		float PanMin;

	/** Maximum angle to pan */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Limits)
		float PanMax;

	/** Camera Pivot Offset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CameraPivot)
		float CameraOffsetX;

	/** Icon to use for Achivement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Data)
		FString CraneModelPath;
};