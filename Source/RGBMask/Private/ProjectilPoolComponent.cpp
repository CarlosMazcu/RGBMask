#include "ProjectilPoolComponent.h"
#include "MaskVisibilitySubsystem.h"
#include "MaskVisibilityComponent.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "TimerManager.h"

UProjectilPoolComponent::UProjectilPoolComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UProjectilPoolComponent::BeginPlay()
{
	Super::BeginPlay();
	Prewarm(InitialSize);
}

void UProjectilPoolComponent::Prewarm(int32 Count)
{
	if (!GetWorld() || !ProjectileClass) return;

	for (int32 i = 0; i < Count; ++i)
	{
		if (AActor* A = SpawnOne())
		{
			DeactivateProjectile(A);
			Available.Add(A);
		}
	}
}

AActor* UProjectilPoolComponent::SpawnOne()
{
	if (!GetWorld() || !ProjectileClass) return nullptr;

	FActorSpawnParameters SP;
	SP.Owner = GetOwner();
	SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FTransform T(FRotator::ZeroRotator, FVector(0, 0, -100000), FVector(1));
	return GetWorld()->SpawnActor<AActor>(ProjectileClass, T, SP);
}

AActor* UProjectilPoolComponent::AcquireProjectile(const FProjectileSpawnParams& Params)
{
	if (!ProjectileClass || !GetWorld()) return nullptr;

	AActor* Projectile = nullptr;

	if (Available.Num() > 0)
	{
		Projectile = Available.Pop(EAllowShrinking::No);
	}
	else if (bAllowExpand && ExpandBy > 0)
	{
		Prewarm(ExpandBy);
		if (Available.Num() > 0)
		{
			Projectile = Available.Pop(EAllowShrinking::No);
		}
	}

	if (!Projectile) return nullptr;

	InUse.Add(Projectile);
	ActivateProjectile(Projectile, Params);
	return Projectile;
}

void UProjectilPoolComponent::ReleaseProjectile(AActor* Projectile)
{
	if (!Projectile) return;

	// Guard: avoid double release
	if (!InUse.Contains(Projectile))
	{
		return;
	}

	// Clear lifetime timer if any
	if (UWorld* World = GetWorld())
	{
		if (FTimerHandle* Handle = LifeTimers.Find(Projectile))
		{
			World->GetTimerManager().ClearTimer(*Handle);
			LifeTimers.Remove(Projectile);
		}
	}

	InUse.Remove(Projectile);
	DeactivateProjectile(Projectile);
	Available.Add(Projectile);
}

void UProjectilPoolComponent::DeactivateProjectile(AActor* Projectile)
{
	Projectile->SetActorHiddenInGame(true);
	Projectile->SetActorEnableCollision(false);
	Projectile->SetActorTickEnabled(false);

	// Stop ProjectileMovement if present
	TArray<UProjectileMovementComponent*> Moves;
	Projectile->GetComponents<UProjectileMovementComponent>(Moves);
	for (UProjectileMovementComponent* M : Moves)
	{
		if (!M) continue;
		M->StopMovementImmediately();
		M->Deactivate();
	}

	// Move away to avoid weird overlaps
	Projectile->SetActorLocation(FVector(0, 0, -100000), false, nullptr, ETeleportType::TeleportPhysics);
}

void UProjectilPoolComponent::ActivateProjectile(AActor* Projectile, const FProjectileSpawnParams& Params)
{
	// Clear previous lifetime timer (if any)
	if (UWorld* World = GetWorld())
	{
		if (FTimerHandle* Handle = LifeTimers.Find(Projectile))
		{
			World->GetTimerManager().ClearTimer(*Handle);
			LifeTimers.Remove(Projectile);
		}
	}

	// Place projectile
	Projectile->SetActorTransform(Params.SpawnTransform, false, nullptr, ETeleportType::TeleportPhysics);

	// IMPORTANT:
	// Do NOT force visibility/collision here. Mask system will decide via ApplyMask().
	// We only ensure the projectile logic/movement can run.
	Projectile->SetActorTickEnabled(true);

	// Set initial velocity if it has ProjectileMovement
	TArray<UProjectileMovementComponent*> Moves;
	Projectile->GetComponents<UProjectileMovementComponent>(Moves);
	for (UProjectileMovementComponent* M : Moves)
	{
		if (!M) continue;
		M->Velocity = Params.InitialVelocity;
		M->Activate(true);
	}

	// ---- APPLY CURRENT MASK RIGHT HERE (this is the fix) ----
	if (UWorld* World = GetWorld())
	{
		if (UMaskVisibilitySubsystem* Sub = World->GetSubsystem<UMaskVisibilitySubsystem>())
		{
			if (UMaskVisibilityComponent* MaskComp = Projectile->FindComponentByClass<UMaskVisibilityComponent>())
			{
				// Apply current mask immediately (no FX) so projectiles spawned while masked start hidden
				MaskComp->ApplyMask(Sub->GetCurrentMask(), /*bAllowFX=*/false);
			}
			else
			{
				// Fallback: if projectile has no mask component, at least make it visible/collidable
				Projectile->SetActorHiddenInGame(false);
				Projectile->SetActorEnableCollision(true);
			}
		}
	}

	// Auto-release by lifetime (per projectile)
	if (Params.LifeTime > 0.f && GetWorld())
	{
		FTimerDelegate D;
		D.BindUFunction(this, FName("ReleaseProjectile"), Projectile);

		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, D, Params.LifeTime, false);

		LifeTimers.Add(Projectile, Handle);
	}
}
