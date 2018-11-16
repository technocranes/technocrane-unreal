// Copyright (c) 2018 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// TechnocraneRig.h
// Sergei <Neill3d> Solokhin 2018

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TechnocraneRig.generated.h"


class USkeletalMeshComponent;
class UPoseableMeshComponent;

/** Shake start offset parameter */
UENUM(BlueprintType)
enum class ECranePreviewModelsEnum : uint8
{
	ECranePreview_Technodolly		UMETA(DisplayName = "Technodolly"),
	ECranePreview_Supertechno50		UMETA(DisplayName = "SuperTechno 50 Plus")
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
		ECranePreviewModelsEnum			CraneModel;

	/** Controls the pitch of the crane arm. */
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Crane Controls", meta = (Units = cm))
		float TrackPosition;

	/** Controls the amount of rails for the crane rig. */
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Crane Controls", meta = (ClampMin = 0, ClampMax = 6))
		int AmountOfTracks;

	/** Controls the attaced camera pivot offset. */
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Crane Controls", meta = (Units = cm))
		FVector CameraPivotOffset;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditUndo() override;
	virtual void EditorApplyTranslation(const FVector& DeltaTranslation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
#endif
	//virtual class USceneComponent* GetDefaultAttachComponent() const override;

private:
#if WITH_EDITORONLY_DATA
	void UpdatePreviewMeshes();
#endif
	void UpdateCraneComponents();

	/** Root component to give the whole actor a transform. */
	UPROPERTY(EditDefaultsOnly, Category = "Crane Components")
		USceneComponent* TransformComponent;

	/** Camera component to use as a crane target. */
	UPROPERTY(EditAnywhere, Category = "Crane Components")
		FComponentReference		TargetComponent;

#if WITH_EDITORONLY_DATA
	/** Preview meshes for visualization */
	UPROPERTY()
		UPoseableMeshComponent* PreviewMesh_CraneBase;
#endif

private:
	FTechnocraneRig			TechnocraneRig;

	bool					mIsNeckRotationSetted;
};
