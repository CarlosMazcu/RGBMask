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
    bool bShouldBeHidden = false;

    // Determinar si debe estar oculto según el modo
    switch (VisibilityMode)
    {
    case EMaskVisibilityMode::HideInMasks:
        // Modo clásico: ocultar si la máscara está en la lista
        bShouldBeHidden = HiddenInMask.Contains(Mask);
        break;

    case EMaskVisibilityMode::ShowOnlyInMasks:
        // Modo inverso: ocultar si la máscara NO está en la lista
        bShouldBeHidden = !VisibleInMask.Contains(Mask);
        break;
    }

    //const bool bShouldBeHidden = HiddenInMask.Contains(Mask);

    AActor* Owner = GetOwner();
    if (!Owner) return;

    Owner->SetActorHiddenInGame(bShouldBeHidden);

    if (bDisableCollisionWhenHidden)
        Owner->SetActorEnableCollision(!bShouldBeHidden);

    if (bDisableTickWhenHidden)
        Owner->SetActorTickEnabled(!bShouldBeHidden);
}
