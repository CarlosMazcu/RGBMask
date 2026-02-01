// Copyright Epic Games, Inc. All Rights Reserved.

#include "RGBMaskCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "RGBMaskGameInstance.h"   
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "Engine/PostProcessVolume.h"
#include "Materials/MaterialInstance.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"


void ARGBMaskCharacter::Die()
{
	if (bIsDead) return;
	bIsDead = true;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	// Parar movimiento y deshabilitar movement (opcional)
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}
	PushDeathInputBlocker(PC);

	// (Extra) ignora movimiento/look por si acaso
	PC->SetIgnoreMoveInput(true);
	PC->SetIgnoreLookInput(true);

	// Fade a negro
	if (PC->PlayerCameraManager)
	{
		PC->PlayerCameraManager->StartCameraFade(
			0.f, 1.f, DeathFadeDuration, FLinearColor::Black,
			false,
			true
		);
	}

	// Widget YOU DIED
	if (DeathWidgetClass && !DeathWidgetInstance)
	{
		DeathWidgetInstance = CreateWidget<UUserWidget>(PC, DeathWidgetClass);
		if (DeathWidgetInstance)
		{
			DeathWidgetInstance->AddToViewport(9999);
		}
	}

	// Timer -> LoadGame (BP en GameInstance)
	GetWorldTimerManager().SetTimer(
		DeathTimerHandle,
		this,
		&ARGBMaskCharacter::HandleDeathReload,
		DeathReloadDelay,
		false
	);
	OnPlayerDead.Broadcast();
}

void ARGBMaskCharacter::PushDeathInputBlocker(APlayerController* PC)
{
	if (!PC) return;

	if (!DeathInputBlocker)
	{
		DeathInputBlocker = NewObject<UInputComponent>(this, TEXT("DeathInputBlocker"));
		DeathInputBlocker->bBlockInput = true;
		DeathInputBlocker->Priority = 100000; 
		DeathInputBlocker->RegisterComponent();
	}

	PC->PushInputComponent(DeathInputBlocker);
}

void ARGBMaskCharacter::PopDeathInputBlocker(APlayerController* PC)
{
	if (!PC || !DeathInputBlocker) return;

	PC->PopInputComponent(DeathInputBlocker);
	DeathInputBlocker->DestroyComponent();
	DeathInputBlocker = nullptr;
}

void ARGBMaskCharacter::HandleDeathReload()
{
	APlayerController* PC = Cast<APlayerController>(GetController());

	// Llama al LoadGame implementado en BP_GameInstance
	if (URGBMaskGameInstance* GI = GetGameInstance<URGBMaskGameInstance>())
	{
		GI->LoadGame();
	}

	// Restaurar input (si LoadGame hace OpenLevel, esto no molesta; si no cambia de nivel, esto es necesario)
	if (PC)
	{
		PopDeathInputBlocker(PC);

		PC->SetIgnoreMoveInput(false);
		PC->SetIgnoreLookInput(false);

		// Quitar fade si sigues en el mismo nivel
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->StartCameraFade(
				1.f, 0.f, 0.25f, FLinearColor::Black,
				false,
				false
			);
		}
	}

	// Quitar UI
	if (DeathWidgetInstance)
	{
		DeathWidgetInstance->RemoveFromParent();
		DeathWidgetInstance = nullptr;
	}

	// Reactivar movimiento si sigues en el mismo pawn
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
	}

	bIsDead = false;
}


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
	CameraBoom->SetUsingAbsoluteRotation(true); // La rotaci�n es fija
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

	Masks.Add(EMaskType::Blue, false);
	Masks.Add(EMaskType::Red, false);
	Masks.Add(EMaskType::Green, false);
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

	// PostProcessVolume setup
	UE_LOG(LogTemp, Warning, TEXT("========== INITIALIZING POST-PROCESS SYSTEM =========="));

	if (!PostProcessVolume && bUsePostProcessEffects)
	{
		UE_LOG(LogTemp, Log, TEXT("PostProcessVolume not assigned, attempting auto-detection..."));

		if (UWorld* World = GetWorld())
		{
			TArray<AActor*> FoundActors;
			UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), FoundActors);

			UE_LOG(LogTemp, Log, TEXT("Found %d PostProcessVolume(s) in the level"), FoundActors.Num());

			for (AActor* Actor : FoundActors)
			{
				APostProcessVolume* PPV = Cast<APostProcessVolume>(Actor);
				if (PPV && PPV->bUnbound)
				{
					PostProcessVolume = PPV;
					UE_LOG(LogTemp, Warning, TEXT("✓ Auto-detected UNBOUND PostProcessVolume: %s"), *PPV->GetName());
					break;
				}
			}

			if (!PostProcessVolume && FoundActors.Num() > 0)
			{
				PostProcessVolume = Cast<APostProcessVolume>(FoundActors[0]);
				UE_LOG(LogTemp, Warning, TEXT("⚠ Using first PostProcessVolume (NOT unbound): %s"),
					*PostProcessVolume->GetName());
				UE_LOG(LogTemp, Warning, TEXT("  Consider setting bUnbound=true on this volume for global effect"));
			}

			if (!PostProcessVolume)
			{
				UE_LOG(LogTemp, Error, TEXT("✗ No PostProcessVolume found in level! Post-process effects will not work."));
				UE_LOG(LogTemp, Error, TEXT("  Please add a PostProcessVolume to your level and set bUnbound=true"));
			}
		}
	}
	else if (PostProcessVolume)
	{
		UE_LOG(LogTemp, Warning, TEXT("✓ Using manually assigned PostProcessVolume: %s"), *PostProcessVolume->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Post-process effects disabled (bUsePostProcessEffects = false)"));
	}

	UE_LOG(LogTemp, Warning, TEXT("======================================================"));

	// Apply initial post process effect
	UpdatePostProcess();

	// Print debug info
	DebugPrintPostProcessInfo();
}

void ARGBMaskCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// stub
}

//void ARGBMaskCharacter::SetMask(EMaskType NewMask)
//{
//
//	if (CurrentMask == NewMask) return;
//	CurrentMask = NewMask;
//	TObjectPtr<UMaterialInterface> ChosenMat;
//
//	switch (CurrentMask)
//	{
//	case EMaskType::Red:
//		ChosenMat = RedMaskMaterial;
//		break;
//	case EMaskType::Green:
//		ChosenMat = GreenMaskMaterial;
//		break;
//	case EMaskType::Blue:
//		ChosenMat = BlueMaskMaterial;
//		break;
//	case EMaskType::None:
//		ChosenMat = MainMaterial;
//		break;
//	default:
//		break;
//	}
//	if (USkeletalMeshComponent* MeshComp = GetMesh())
//	{
//		const int32 NumMats = MeshComp->GetNumMaterials();
//		if (NumMats <= 0) return;
//
//		const int32 SafeIndex = FMath::Clamp(MaskMaterialIndex, 0, NumMats - 1);
//		MeshComp->SetMaterial(0, ChosenMat);
//	}
//	OnMaskChanged.Broadcast(CurrentMask);
//
//}

void ARGBMaskCharacter::SetMask(EMaskType NewMask)
{
	if (bIsMaskChangeInProgress && PendingMask == NewMask) return;

	if (!bIsMaskChangeInProgress && CurrentMask == NewMask) return;

	if (bIsMaskChangeInProgress)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(MaskChangeTimerHandle);
		}
		bIsMaskChangeInProgress = false;
	}

	PendingMask = NewMask;
	bIsMaskChangeInProgress = true;

	OnMaskChangeStarted.Broadcast(NewMask);

	if (bUseSmoothBlending && bUsePostProcessEffects)
	{
		StartPostProcessBlend();
	}

	if (MaskChangeDelay <= 0.0f)
	{
		ApplyMaskChange();
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			MaskChangeTimerHandle,
			this,
			&ARGBMaskCharacter::ApplyMaskChange,
			MaskChangeDelay,
			false
		);
	}
}

void ARGBMaskCharacter::ApplyMaskChange()
{
	bIsMaskChangeInProgress = false;

	// Si la m�scara pendiente es la misma que la actual, solo limpiamos el flag
	if (CurrentMask == PendingMask) return;

	CurrentMask = PendingMask;
	/*TObjectPtr<UMaterialInterface> ChosenMat;

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
	}*/
	OnMaskChanged.Broadcast(CurrentMask);

	// Update post process effect based on new mask (only if NOT using smooth blending)
	if (!bUseSmoothBlending)
	{
		UpdatePostProcess();
	}

}

void ARGBMaskCharacter::UpdatePostProcess()
{
	// Early exit if post process effects are disabled or volume is not assigned
	if (!bUsePostProcessEffects)
	{
		UE_LOG(LogTemp, Warning, TEXT("PostProcess disabled via bUsePostProcessEffects"));
		return;
	}

	if (!PostProcessVolume)
	{
		UE_LOG(LogTemp, Error, TEXT("PostProcessVolume is NULL! Cannot apply post-process."));
		return;
	}

	TObjectPtr<UMaterialInstance> ChosenPostProcessMat = nullptr;

	// Select the appropriate post process material based on current mask
	switch (CurrentMask)
	{
	case EMaskType::Red:
		ChosenPostProcessMat = RedPostProcessMaterial;
		UE_LOG(LogTemp, Log, TEXT("Selecting RED post-process material"));
		break;
	case EMaskType::Green:
		ChosenPostProcessMat = GreenPostProcessMaterial;
		UE_LOG(LogTemp, Log, TEXT("Selecting GREEN post-process material"));
		break;
	case EMaskType::Blue:
		ChosenPostProcessMat = BluePostProcessMaterial;
		UE_LOG(LogTemp, Log, TEXT("Selecting BLUE post-process material"));
		break;
	case EMaskType::None:
		ChosenPostProcessMat = NonePostProcessMaterial;
		UE_LOG(LogTemp, Log, TEXT("Selecting NONE (default) post-process material"));
		break;
	default:
		ChosenPostProcessMat = NonePostProcessMaterial;
		UE_LOG(LogTemp, Warning, TEXT("Unknown mask type, using NONE post-process material"));
		break;
	}

	// Apply the post process material if valid
	if (ChosenPostProcessMat)
	{
		PostProcessVolume->Settings.WeightedBlendables.Array.Empty();
		PostProcessVolume->Settings.WeightedBlendables.Array.Add(FWeightedBlendable(PostProcessBlendWeight, ChosenPostProcessMat));

		UE_LOG(LogTemp, Log, TEXT("Applied post-process material: %s with weight: %f"),
			*ChosenPostProcessMat->GetName(), PostProcessBlendWeight);
	}
	else
	{
		// Clear post process effects if no material is assigned
		PostProcessVolume->Settings.WeightedBlendables.Array.Empty();
		UE_LOG(LogTemp, Warning, TEXT("No post-process material assigned for current mask. Clearing effects."));
	}
}

void ARGBMaskCharacter::StartPostProcessBlend()
{
	if (!PostProcessVolume || !bUsePostProcessEffects)
		return;

	// Save the previous mask
	PreviousMask = CurrentMask;
	PostProcessBlendAlpha = 0.0f;
	bIsBlendingPostProcess = true;

	// Start the blend timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			PostProcessBlendTimerHandle,
			this,
			&ARGBMaskCharacter::TickPostProcessBlend,
			0.016f, // ~60 FPS
			true
		);
	}
}

void ARGBMaskCharacter::TickPostProcessBlend()
{
	if (!bIsBlendingPostProcess || !PostProcessVolume)
		return;

	// Increment blend alpha
	if (UWorld* World = GetWorld())
	{
		PostProcessBlendAlpha += World->GetDeltaSeconds() / FMath::Max(PostProcessBlendDuration, 0.01f);
	}

	// Clamp to 0-1 range
	PostProcessBlendAlpha = FMath::Clamp(PostProcessBlendAlpha, 0.0f, 1.0f);

	// Get materials for previous and current masks
	TObjectPtr<UMaterialInstance> PrevMat = nullptr;
	TObjectPtr<UMaterialInstance> CurrMat = nullptr;

	// Get previous mask material
	switch (PreviousMask)
	{
	case EMaskType::Red:
		PrevMat = RedPostProcessMaterial;
		break;
	case EMaskType::Green:
		PrevMat = GreenPostProcessMaterial;
		break;
	case EMaskType::Blue:
		PrevMat = BluePostProcessMaterial;
		break;
	case EMaskType::None:
		PrevMat = NonePostProcessMaterial;
		break;
	}

	// Get current mask material (pending)
	switch (PendingMask)
	{
	case EMaskType::Red:
		CurrMat = RedPostProcessMaterial;
		break;
	case EMaskType::Green:
		CurrMat = GreenPostProcessMaterial;
		break;
	case EMaskType::Blue:
		CurrMat = BluePostProcessMaterial;
		break;
	case EMaskType::None:
		CurrMat = NonePostProcessMaterial;
		break;
	}

	// Apply blended materials
	PostProcessVolume->Settings.WeightedBlendables.Array.Empty();

	if (PrevMat)
	{
		float PrevWeight = (1.0f - PostProcessBlendAlpha) * PostProcessBlendWeight;
		PostProcessVolume->Settings.WeightedBlendables.Array.Add(FWeightedBlendable(PrevWeight, PrevMat));
	}

	if (CurrMat)
	{
		float CurrWeight = PostProcessBlendAlpha * PostProcessBlendWeight;
		PostProcessVolume->Settings.WeightedBlendables.Array.Add(FWeightedBlendable(CurrWeight, CurrMat));
	}

	// Check if blend is complete
	if (PostProcessBlendAlpha >= 1.0f)
	{
		bIsBlendingPostProcess = false;

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(PostProcessBlendTimerHandle);
		}

		// Set final state
		UpdatePostProcess();
	}
}

void ARGBMaskCharacter::AddMaskToInventory(EMaskType mask)
{
	Masks[mask] = true;
}

void ARGBMaskCharacter::DeleteMask(EMaskType mask)
{
	Masks[mask] = false;

}

void ARGBMaskCharacter::DebugPrintPostProcessInfo()
{
	UE_LOG(LogTemp, Warning, TEXT("========== POST-PROCESS DEBUG INFO =========="));
	UE_LOG(LogTemp, Warning, TEXT("Current Mask: %d"), (int)CurrentMask);
	UE_LOG(LogTemp, Warning, TEXT("Pending Mask: %d"), (int)PendingMask);
	UE_LOG(LogTemp, Warning, TEXT("bUsePostProcessEffects: %s"), bUsePostProcessEffects ? TEXT("TRUE") : TEXT("FALSE"));
	UE_LOG(LogTemp, Warning, TEXT("bUseSmoothBlending: %s"), bUseSmoothBlending ? TEXT("TRUE") : TEXT("FALSE"));
	UE_LOG(LogTemp, Warning, TEXT("PostProcessBlendWeight: %f"), PostProcessBlendWeight);

	if (PostProcessVolume)
	{
		UE_LOG(LogTemp, Warning, TEXT("PostProcessVolume: %s (VALID)"), *PostProcessVolume->GetName());
		UE_LOG(LogTemp, Warning, TEXT("  - bUnbound: %s"), PostProcessVolume->bUnbound ? TEXT("TRUE") : TEXT("FALSE"));
		UE_LOG(LogTemp, Warning, TEXT("  - Priority: %f"), PostProcessVolume->Priority);
		UE_LOG(LogTemp, Warning, TEXT("  - BlendWeight: %f"), PostProcessVolume->BlendWeight);
		UE_LOG(LogTemp, Warning, TEXT("  - Num Blendables: %d"), PostProcessVolume->Settings.WeightedBlendables.Array.Num());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PostProcessVolume: NULL"));
	}

	UE_LOG(LogTemp, Warning, TEXT("Materials Assigned:"));
	UE_LOG(LogTemp, Warning, TEXT("  - Red: %s"), RedPostProcessMaterial ? *RedPostProcessMaterial->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("  - Green: %s"), GreenPostProcessMaterial ? *GreenPostProcessMaterial->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("  - Blue: %s"), BluePostProcessMaterial ? *BluePostProcessMaterial->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("  - None: %s"), NonePostProcessMaterial ? *NonePostProcessMaterial->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("============================================="));
}

void ARGBMaskCharacter::ForceUpdatePostProcess()
{
	UE_LOG(LogTemp, Warning, TEXT("FORCING PostProcess Update..."));
	UpdatePostProcess();
}