// Fill out your copyright notice in the Description page of Project Settings.

#include "MaskVisibilitySubsystem.h"
#include "MaskVisibilityComponent.h"
#include "RGBMaskCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

void UMaskVisibilitySubsystem::Register(UMaskVisibilityComponent* Comp)
{
    if (!Comp) return;

    Registered.Add(Comp);

    EnsureBoundToPlayer();

   Comp->ApplyMask(CurrentMask);
}

void UMaskVisibilitySubsystem::Unregister(UMaskVisibilityComponent* Comp)
{
    if (!Comp) return;
    Registered.Remove(Comp);
}

void UMaskVisibilitySubsystem::EnsureBoundToPlayer()
{
    if (bBoundToPlayer && CachedPlayer.IsValid())
        return;

    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC) { ScheduleBindRetry(); return; }

    ARGBMaskCharacter* Player = Cast<ARGBMaskCharacter>(PC->GetPawn());
    if (!Player) { ScheduleBindRetry(); return; }

    CachedPlayer = Player;
    Player->OnMaskChanged.AddUObject(this, &UMaskVisibilitySubsystem::OnPlayerMaskChanged);
    bBoundToPlayer = true;

    World->GetTimerManager().ClearTimer(RetryBindHandle);

    CurrentMask = Player->GetMask();
    ApplyMaskToAll();
}


void UMaskVisibilitySubsystem::OnPlayerMaskChanged(EMaskType NewMask)
{
    CurrentMask = NewMask;
    ApplyMaskToAll();
}

void UMaskVisibilitySubsystem::PruneInvalid()
{
    for (auto It = Registered.CreateIterator(); It; ++It)
    {
        if (!It->IsValid())
        {
            It.RemoveCurrent();
        }
    }
}

void UMaskVisibilitySubsystem::ApplyMaskToAll()
{
    PruneInvalid();

    for (const TWeakObjectPtr<UMaskVisibilityComponent>& WeakComp : Registered)
    {
        if (UMaskVisibilityComponent* Comp = WeakComp.Get())
        {
            Comp->ApplyMask(CurrentMask);
        }
    }
}

void UMaskVisibilitySubsystem::ScheduleBindRetry()
{
    if (bBoundToPlayer) return;

    if (UWorld* World = GetWorld())
    {
        if (!World->GetTimerManager().IsTimerActive(RetryBindHandle))
        {
            World->GetTimerManager().SetTimer(
                RetryBindHandle,
                this,
                &UMaskVisibilitySubsystem::EnsureBoundToPlayer,
                0.1f,
                true
            );
        }
    }
}


