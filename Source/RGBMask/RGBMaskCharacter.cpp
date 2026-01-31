// Copyright Epic Games, Inc. All Rights Reserved.

#include "RGBMaskCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "Engine/World.h"

ARGBMaskCharacter::ARGBMaskCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 10000.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create the camera boom component
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));

	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false;

	// Create the camera component
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));

	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false;

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	Masks.Reserve(3);

	Masks.Add(EMaskType::Blue, true);
	Masks.Add(EMaskType::Red, true);
	Masks.Add(EMaskType::Green, true);
}

void ARGBMaskCharacter::BeginPlay()
{
	Super::BeginPlay();
	OnMaskChanged.Broadcast(CurrentMask);
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		const int32 NumMats = MeshComp->GetNumMaterials();
		if (NumMats <= 0) return;

		const int32 SafeIndex = FMath::Clamp(MaskMaterialIndex, 0, NumMats - 1);
		MainMaterial = MeshComp->GetMaterial(SafeIndex);
	}
	// stub
}

void ARGBMaskCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

	// stub
}

void ARGBMaskCharacter::SetMask(EMaskType NewMask)
{
	if (CurrentMask == NewMask) return;
	CurrentMask = NewMask;
	TObjectPtr<UMaterialInterface> ChosenMat;

	switch (CurrentMask)
	{
	case EMaskType::Red:
		ChosenMat = RedMaskMaterial;
		break;
	case EMaskType::Green:
		ChosenMat = GreenMaskMaterial;
		break;
	case EMaskType::Blue:
		ChosenMat = BlueMaskMaterial;
		break;
	case EMaskType::None:
		ChosenMat = MainMaterial;
		break;
	default:
		break;
	}
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		const int32 NumMats = MeshComp->GetNumMaterials();
		if (NumMats <= 0) return;

		const int32 SafeIndex = FMath::Clamp(MaskMaterialIndex, 0, NumMats - 1);
		MeshComp->SetMaterial(0, ChosenMat);
	}
	OnMaskChanged.Broadcast(CurrentMask);

}

void ARGBMaskCharacter::AddMaskToInventory(EMaskType mask)
{
	Masks[mask] = true;
}

void ARGBMaskCharacter::DeleteMask(EMaskType mask)
{
	Masks[mask] = false;

}
