// Copyright (c) 2020-2025 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// TechnocraneRig.h
// Sergei <Neill3d> Solokhin

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "TechnocraneData.h"
#include "TechnocraneRig.generated.h"

class USkeletalMeshComponent;
class UPoseableMeshComponent;

/** Shake start offset parameter */
UENUM(BlueprintType)
enum class ECranePreviewModelsEnum : uint8
{
	ECranePreview_Technodolly10 = 0		UMETA(DisplayName = "Technodolly 10"),
	ECranePreview_Technodolly15 = 1		UMETA(DisplayName = "Technodolly 15"),
	ECranePreview_Technodolly25 = 2		UMETA(DisplayName = "Technodolly 25"),
	ECranePreview_Technocrane10 = 3		UMETA(DisplayName = "Technocrane 10"),
	ECranePreview_Technocrane22 = 4		UMETA(DisplayName = "Technocrane 22"),
	ECranePreview_Supertechno15 = 5		UMETA(DisplayName = "SuperTechno 15"),
	ECranePreview_Supertechno30 = 6		UMETA(DisplayName = "SuperTechno 30 Plus"),
	ECranePreview_Supertechno50 = 7		UMETA(DisplayName = "SuperTechno 50 Plus"),
	ECranePreview_Supertechno75 = 8		UMETA(DisplayName = "SuperTechno 75 Plus"),
	ECranePreview_Count					UMETA(Hidden)
};

class FTechnocraneRigImpl;

class FTechnocraneRig
{
public:
	FTechnocraneRig();
	~FTechnocraneRig();

public:
	/** This is a private data **/
	TUniquePtr<FTechnocraneRigImpl> CreateImpl();
	TUniquePtr<FTechnocraneRigImpl> Impl;
};

/**
 * an actor that simulates a Technocrane Crane Rig to follow a target camera position
 *  in case tracks are used, the position on track could be received from live or exported tecnocrane data
*/
UCLASS(Blueprintable, ClassGroup = "Technocrane", meta = (BlueprintSpawnableComponent))
class TECHNOCRANEPLUGIN_API ATechnocraneRig : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATechnocraneRig();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldTickIfViewportsOnly() const override;

	/** Defines how to begin (either at zero, or at a randomized value. */
	UPROPERTY(EditAnywhere, Category = "Crane Controls")
	ECranePreviewModelsEnum			CraneModel{ ECranePreviewModelsEnum::ECranePreview_Technodolly25 };

	/** Crane position on tracks, reads from Camera Component or from Live Data */
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Crane Controls", meta = (Units = cm))
	float TrackPosition{ 0.0f };

	/** Controls if we would like to show a preview of tracks (only when crane supports tracks preview) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crane Controls")
	bool bShowTracksIfSupported{ true };

	/** Controls the amount of rails for the crane rig. */
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Crane Controls", meta = (ClampMin = 0, ClampMax = 6))
	int AmountOfTracks{ 0 };

	/** Controls the attaced camera pivot offset. */
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Crane Controls", meta = (Units = cm))
	FVector CameraPivotOffset;

	/** Display debug primitives for the running technocrane rig simulation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crane Controls")
	bool bShowDebug{ false };

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditUndo() override;
	virtual void EditorApplyTranslation(const FVector& DeltaTranslation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
#endif

public:

	/** some calculated output from a crane simulation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "Output", meta = (Units = cm))
	FCraneSimulationData	SimulationData;

private:
#if WITH_EDITORONLY_DATA
	bool PreloadPreviewMeshes();
	void UpdatePreviewMeshes();
	bool PreloadTracksMesh();
	void UpdateTracksMesh();
#endif
	void UpdateCraneComponents();

	/** Root component to give the whole actor a transform. */
	UPROPERTY(EditDefaultsOnly, Category = "Crane Components")
	USceneComponent* TransformComponent{ nullptr };

	/** Camera component to use as a crane target. */
	UPROPERTY(EditAnywhere, Category = "Crane Components")
	FComponentReference		TargetComponent;

#if WITH_EDITORONLY_DATA
	/** Preview meshes for visualization */
	UPROPERTY()
	TArray<USkeletalMesh*> PreviewMeshes;
	
	UPROPERTY()
	USkeletalMeshComponent* MeshComponent{ nullptr };
	//UPoseableMeshComponent* MeshComponent{ nullptr };
	
	/** Preview mesh of crane tracks */
	UPROPERTY()
	UStaticMesh* CraneTracksMesh{ nullptr };

	/** mesh component for tracks */
	UPROPERTY()
	UStaticMeshComponent* CraneTracksMeshComponent{ nullptr };

	/** Data table with crane presets, the asset is part of technocrane plugin content */
	UPROPERTY()
	UDataTable* CranesData{ nullptr };
#endif

private:
	FTechnocraneRig				TechnocraneRig;
	ECranePreviewModelsEnum		LastPreviewModel;
	bool						bLastSupportTracks{ false };
	float						LastTracksOffset{ 0.0f };
};
