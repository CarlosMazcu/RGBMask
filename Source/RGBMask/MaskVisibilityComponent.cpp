#include "MaskVisibilityComponent.h"
#include "MaskVisibilitySubsystem.h"
#include "Engine/World.h"

void UMaskVisibilityComponent::BeginPlay()
{
    Super::BeginPlay();

    if (UWorld* World = GetWorld())
    {
        if (UMaskVisibilitySubsystem* Sub = World->GetSubsystem<UMaskVisibilitySubsystem>())
        {
            Sub->Register(this);
        }
    }
}

void UMaskVisibilityComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UWorld* World = GetWorld())
    {
        if (UMaskVisibilitySubsystem* Sub = World->GetSubsystem<UMaskVisibilitySubsystem>())
        {
            Sub->Unregister(this);
        }
    }

    Super::EndPlay(EndPlayReason);
}

void UMaskVisibilityComponent::ApplyMask(EMaskType Mask)
{
    const bool bShouldBeHidden = HiddenInMask.Contains(Mask);

    AActor* Owner = GetOwner();
    if (!Owner) return;

    Owner->SetActorHiddenInGame(bShouldBeHidden);

    if (bDisableCollisionWhenHidden)
        Owner->SetActorEnableCollision(!bShouldBeHidden);

    if (bDisableTickWhenHidden)
        Owner->SetActorTickEnabled(!bShouldBeHidden);
}
