// Copyright (c) 2018 Technocrane s.r.o. 
//
// TechnocraneRig.cpp
// Sergei <Neill3d> Solokhin 2018

#include "TechnocranePrivatePCH.h"
#include "TechnocraneRig.h"
#include <UObject/ConstructorHelpers.h>
#include <Components/SkeletalMeshComponent.h>
#include <Components/PoseableMeshComponent.h>
#include <Engine/SkeletalMesh.h>
#include "Engine/CollisionProfile.h"
#include "DrawDebugHelpers.h"

#include <Runtime/CinematicCamera/Public/CineCameraActor.h>
#include <Runtime/CinematicCamera/Public/CineCameraComponent.h>

#include <IPluginManager.h>
#include <Paths.h>

#include "TechnocraneShared.h"
#include <Public/TechnocraneCamera.h>

#define LOCTEXT_NAMESPACE "TechnocraneRig_Crane"


///////////////////////////////////////////////////////////////////////////////////
// FTechnocraneRigImpl

class FTechnocraneRigImpl
{
public:

	//! a constructor
	FTechnocraneRigImpl()
	{
		mComponent = nullptr;
	}

	UPoseableMeshComponent*		mComponent;

	
	void SetPoseableMeshComponent(UPoseableMeshComponent* component)
	{
		mComponent = component;
	}

	FVector ComputeHeadTransform()
	{
		const FName bone_name = GetCraneJointName(ECraneJoints::Head);
		return mComponent->GetBoneLocation(bone_name, EBoneSpaces::WorldSpace);
	}

	bool Reset()
	{
		mComponent->ResetBoneTransformByName(GetCraneJointName(ECraneJoints::Columns));
		mComponent->ResetBoneTransformByName(GetCraneJointName(ECraneJoints::Beams));
		return true;
	}

	// reompute crane transform to fit a target
	bool Compute(UWorld* pWorld, const FTransform& base, const FVector& target, const float track_position, const FVector& raw_rotation, const FQuat& neck_rotation)
	{
		const float ZOffset = 36.f;		// make it appear in the right place
		FVector const NewLoc(0.0f, track_position, ZOffset);
		
		

		const FTransform parentTM = base;
		const FTransform relativeTM = parentTM.GetRelativeTransformReverse(FTransform(target));
		FVector vTarget3 = target; // relativeTM.GetLocation();

		const FName root_name = GetCraneJointName(ECraneJoints::Base);
		const FName columns_name = GetCraneJointName(ECraneJoints::Columns);
		const FName beams_name = GetCraneJointName(ECraneJoints::Beams);
		const FName gravity_name = GetCraneJointName(ECraneJoints::Gravity);
		const FName beam1_name = GetCraneJointName(ECraneJoints::Beam1);
		const FName beam2_name = GetCraneJointName(ECraneJoints::Beam2);
		const FName beam4_name = GetCraneJointName(ECraneJoints::Beam4);
		const FName neck_name(GetCraneJointName(ECraneJoints::Neck));
		const FName head_name(GetCraneJointName(ECraneJoints::Head));

		mComponent->SetBoneLocationByName(root_name, NewLoc, EBoneSpaces::ComponentSpace);

		FVector vHeadPos;
		FVector vColumn, vBeams, projPos;
		FVector vHeadRight;
		FPlane	plane;

		FQuat DeltaQuat;
		FVector DeltaAxis(0.f);
		float DeltaAngle = 0.f;

		const int32 iterations = 32;

		for (int32 iter_id = 0; iter_id < iterations; ++iter_id)
		{
			vHeadPos = ComputeHeadTransform();

			vColumn = mComponent->GetBoneLocation(columns_name);
			vBeams = mComponent->GetBoneLocation(beams_name);

			plane = FPlane(vColumn, vBeams, vHeadPos);

			vHeadRight = vBeams - 150.0f * plane.GetSafeNormal();

			projPos = FVector::PointPlaneProject(vTarget3, vBeams, vHeadRight, vHeadPos);

			bool is_behind = (plane.PlaneDot(vTarget3) < 0.0f);

			// Find delta rotation between both normals.
			DeltaQuat = FQuat::FindBetween(vHeadPos - vBeams, projPos - vBeams);
			DeltaQuat.ToAxisAndAngle(DeltaAxis, DeltaAngle);
			
			if (DeltaAngle == DeltaAngle && abs(DeltaAngle) > 0.01f)
			{
				//UE_LOG(LogTemp, Warning, TEXT("angle - %f"), DeltaAngle)

				FRotator rot = mComponent->GetBoneRotationByName(columns_name, EBoneSpaces::ComponentSpace);

				rot.Yaw += (is_behind) ? -DeltaAngle : DeltaAngle;
				mComponent->SetBoneRotationByName(columns_name, rot, EBoneSpaces::ComponentSpace);
			}

			//
			// up / down

			vHeadPos = ComputeHeadTransform();

			plane = FPlane(vColumn, vBeams, vHeadPos);
			vHeadRight = vBeams - 150.0f * plane.GetSafeNormal();

			plane = FPlane(vBeams, vHeadRight, vHeadPos);
			projPos = FVector::PointPlaneProject(vTarget3, vBeams, vColumn, vHeadPos);

			is_behind = (plane.PlaneDot(vTarget3) < 0.0f);

			// Find delta rotation between both normals.
			DeltaQuat = FQuat::FindBetween(vHeadPos - vBeams, projPos - vBeams);
			DeltaQuat.ToAxisAndAngle(DeltaAxis, DeltaAngle);

			if (DeltaAngle == DeltaAngle && abs(DeltaAngle) > 0.01f)
			{
				FRotator rot = mComponent->GetBoneRotationByName(beams_name, EBoneSpaces::ComponentSpace);

				if (is_behind)
					DeltaAngle = -DeltaAngle;

				rot.Roll += DeltaAngle;
				double angle = rot.Roll;

				mComponent->SetBoneRotationByName(beams_name, rot, EBoneSpaces::ComponentSpace);
			}
		}

		//
		// rotate gravity point
		FTransform tm, parent_tm;
		FVector angles;

		tm = mComponent->GetBoneTransformByName(beams_name, EBoneSpaces::ComponentSpace);
		parent_tm = mComponent->GetBoneTransformByName(mComponent->GetParentBone(beams_name), EBoneSpaces::ComponentSpace);

		tm.SetToRelativeTransform(parent_tm);
		angles = tm.GetRotation().Euler();

		tm = mComponent->GetBoneTransformByName(gravity_name, EBoneSpaces::ComponentSpace);
		parent_tm = mComponent->GetBoneTransformByName(mComponent->GetParentBone(gravity_name), EBoneSpaces::ComponentSpace);

		tm.SetToRelativeTransform(parent_tm);
		tm.SetRotation(FQuat::MakeFromEuler(-angles));

		tm = tm * parent_tm;
		mComponent->SetBoneTransformByName(gravity_name, tm, EBoneSpaces::ComponentSpace);

		//
		// beams length
		
		vHeadPos = ComputeHeadTransform();

		DeltaQuat = FQuat::FindBetween(vHeadPos - vBeams, projPos - vBeams);
		DeltaQuat.ToAxisAndAngle(DeltaAxis, DeltaAngle);

		if (DeltaAngle < 60.0f)
		{
			const float l1 = (vHeadPos - vBeams).Size();
			const float l2 = (vTarget3 - vBeams).Size();
			const float l = 0.15f * (l1 - l2);

			if (abs(l) > 0.1f)
			{
				FTransform parent_tm = mComponent->GetBoneTransformByName(beam1_name, EBoneSpaces::ComponentSpace);

				const int32 beam2 = static_cast<int32>(ECraneJoints::Beam2);
				const int32 beam4 = static_cast<int32>(ECraneJoints::Beam4);

				for (int32 i = beam2; i <= beam4; ++i)
				{
					const FName bone_name(GetCraneJointName(static_cast<ECraneJoints>(i)));
					
					if (INDEX_NONE == mComponent->GetBoneIndex(bone_name))
						continue;

					FTransform tm = mComponent->GetBoneTransformByName(bone_name, EBoneSpaces::ComponentSpace);
					tm.SetToRelativeTransform(parent_tm);
						
					FVector tr = tm.GetLocation();
					tr.Z += l;

					if (tr.Z > -50.0f) tr.Z = -50.0f;
					else if (tr.Z < -400.0f) tr.Z = -400.0f;

					tm.SetLocation(tr);
					tm = tm * parent_tm;
						
					mComponent->SetBoneTransformByName(bone_name, tm, EBoneSpaces::ComponentSpace);

					parent_tm = tm;	
				}
			}
		}
		
		//
		// crane head and neck

		tm = mComponent->GetBoneTransformByName(neck_name, EBoneSpaces::ComponentSpace);
		parent_tm = mComponent->GetBoneTransformByName(mComponent->GetParentBone(neck_name), EBoneSpaces::ComponentSpace);

		tm.SetToRelativeTransform(parent_tm);
		
		tm = tm * parent_tm;
		tm.SetRotation(neck_rotation);

		mComponent->SetBoneTransformByName(neck_name, tm, EBoneSpaces::ComponentSpace);

		//

		tm = mComponent->GetBoneTransformByName(head_name, EBoneSpaces::ComponentSpace);
		parent_tm = mComponent->GetBoneTransformByName(mComponent->GetParentBone(head_name), EBoneSpaces::ComponentSpace);

		tm.SetToRelativeTransform(parent_tm);
		tm.SetRotation(FQuat::MakeFromEuler(FVector(raw_rotation.Y, 0.0f, 0.0f)));

		tm = tm * parent_tm;
		mComponent->SetBoneTransformByName(head_name, tm, EBoneSpaces::ComponentSpace);

		return true;
	}
};

FTechnocraneRig::FTechnocraneRig()
{
	// allocate memory for a private data
	Impl = CreateImpl();
}
FTechnocraneRig::~FTechnocraneRig()
{
}

TUniquePtr<FTechnocraneRigImpl> FTechnocraneRig::CreateImpl()
{
	return TUniquePtr<FTechnocraneRigImpl>(new FTechnocraneRigImpl());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sets default values
ATechnocraneRig::ATechnocraneRig()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// default control values
	TrackPosition = 0.0f;
	AmountOfTracks = 1;

	CameraPivotOffset = FVector(-70.0f, 0.0f, 0.0f);

	// create the root component
	TransformComponent = CreateDefaultSubobject<USceneComponent>(TEXT("TransformComponent"));
	RootComponent = TransformComponent;

#if WITH_EDITORONLY_DATA
	// create preview meshes
	if (!IsRunningDedicatedServer())
	{
		// DONE: how to deal with a plugin assets !
		
		const FString CameraPath = TEXT("/TechnocranePlugin/TechnodollyModel");
		static ConstructorHelpers::FObjectFinder<USkeletalMesh> CraneBaseMesh(*CameraPath);
		PreviewMesh_CraneBase = CreateOptionalDefaultSubobject<UPoseableMeshComponent>(TEXT("PreviewMesh_CraneBase"));
		if (PreviewMesh_CraneBase)
		{
			PreviewMesh_CraneBase->SetSkeletalMesh(CraneBaseMesh.Object);
			PreviewMesh_CraneBase->bIsEditorOnly = true;
			PreviewMesh_CraneBase->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
			PreviewMesh_CraneBase->bHiddenInGame = false;
			PreviewMesh_CraneBase->CastShadow = true;
			PreviewMesh_CraneBase->PostPhysicsComponentTick.bCanEverTick = false;
			
			PreviewMesh_CraneBase->SetupAttachment(TransformComponent);		// sibling of yawcontrol

			TechnocraneRig.Impl->SetPoseableMeshComponent(PreviewMesh_CraneBase);
		}

		UpdatePreviewMeshes();
	}
#endif
}

#if WITH_EDITORONLY_DATA
void ATechnocraneRig::UpdatePreviewMeshes()
{
	if (PreviewMesh_CraneBase)
	{

		// Perform kinematic updates

		FTransform target = FTransform::Identity;
		target.SetLocation(FVector(0.0f, 0.0f, 100.0f));
		
		FVector raw_rotation = FVector::ZeroVector;

		ATechnocraneCamera* myCineCamera = Cast<ATechnocraneCamera>(TargetComponent.OtherActor);
		if (myCineCamera != nullptr)
		{
			FTransform camera_transform = myCineCamera->GetTransform();

			if (UCineCameraComponent* comp = myCineCamera->GetCineCameraComponent())
			{
				 camera_transform = comp->GetRelativeTransform() * camera_transform;
			}

			// add some camera pivot offset
			FTransform pivot_offset;
			pivot_offset.SetLocation(CameraPivotOffset);

			target = pivot_offset * camera_transform;
			
			// check for track position
			
			TrackPosition = myCineCamera->TrackPosition;
			raw_rotation = myCineCamera->RawRotation;
			
		}
			
		const FTransform& base = PreviewMesh_CraneBase->GetComponentTransform();
		const float track_position = TrackPosition;
		const FQuat neck_q = FQuat::MakeFromEuler(FVector(90.0f, 0.0f, 180.0f + raw_rotation.X));
		
		TechnocraneRig.Impl->Compute(GetWorld(), base, target.GetLocation(), track_position, raw_rotation, neck_q);
	}
}
#endif

void ATechnocraneRig::UpdateCraneComponents()
{

#if WITH_EDITORONLY_DATA
	UpdatePreviewMeshes();
#endif
}

// Called when the game starts or when spawned
void ATechnocraneRig::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ATechnocraneRig::Tick(float DeltaTime)
{
	mIsNeckRotationSetted = false;

	Super::Tick(DeltaTime);

	// feed exposed API into underlying components
	UpdateCraneComponents();
}

#if WITH_EDITOR
void ATechnocraneRig::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty && PropertyChangedEvent.MemberProperty->GetFName() == "TargetComponent")
	{
		TechnocraneRig.Impl->Reset();
	}

	UpdateCraneComponents();
}

void ATechnocraneRig::PostEditUndo()
{
	Super::PostEditUndo();

	UpdateCraneComponents();
}

void ATechnocraneRig::EditorApplyTranslation(const FVector& DeltaTranslation, bool bAltDown, bool bShiftDown, bool bCtrlDown)
{
	Super::EditorApplyTranslation(DeltaTranslation, bAltDown, bShiftDown, bCtrlDown);

	UpdateCraneComponents();
}

#endif // WITH_EDITOR

bool ATechnocraneRig::ShouldTickIfViewportsOnly() const
{
	return true;
}

#undef LOCTEXT_NAMESPACE
