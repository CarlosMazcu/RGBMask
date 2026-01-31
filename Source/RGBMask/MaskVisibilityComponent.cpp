#include "MaskVisibilityComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "MaskVisibilitySubsystem.h"
#include "Engine/World.h"


UMaskVisibilityComponent::UMaskVisibilityComponent()
{
    static ConstructorHelpers::FObjectFinder<UNiagaraSystem> FX(
        TEXT("/Game/Art/VFX/NS_MaskHide.NS_MaskHide")
    );
    if (FX.Succeeded())
    {
        HideFX = FX.Object;
    }

}

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
    if (ActiveHideFXComponent)
    {
        ActiveHideFXComponent->DeactivateImmediate();
        ActiveHideFXComponent->DestroyComponent();
        ActiveHideFXComponent = nullptr;
    }

    Super::EndPlay(EndPlayReason);
}



void UMaskVisibilityComponent::ApplyMask(EMaskType Mask)
{
    bool bShouldBeHidden = false;

    switch (VisibilityMode)
    {
    case EMaskVisibilityMode::HideInMasks:
        bShouldBeHidden = HiddenInMask.Contains(Mask);
        break;

    case EMaskVisibilityMode::ShowOnlyInMasks:
        bShouldBeHidden = !VisibleInMask.Contains(Mask);
        break;

    default:
        break;
    }

    AActor* Owner = GetOwner();
    if (!Owner) return;

    // --- FX solo si hay transición ---
    const bool bChangingState = (bShouldBeHidden != bWasHidden);

    if (bChangingState)
    {
        if (bShouldBeHidden)
        {
            if (HideFX && !ActiveHideFXComponent)
            {
                const FVector SpawnLoc = Owner->GetActorLocation() + HideFXOffset;
                const FRotator SpawnRot = Owner->GetActorRotation();

                ActiveHideFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                    GetWorld(),
                    HideFX,
                    SpawnLoc,
                    SpawnRot,
                    FVector(1.f),
                    false,
                    true,
                    ENCPoolMethod::None,
                    true
                );
            }
        }
        else
        {
            if (ActiveHideFXComponent)
            {
                ActiveHideFXComponent->DeactivateImmediate();
                ActiveHideFXComponent->DestroyComponent();
                ActiveHideFXComponent = nullptr;
            }
        }
    }

    Owner->SetActorHiddenInGame(bShouldBeHidden);

    if (bDisableCollisionWhenHidden)
        Owner->SetActorEnableCollision(!bShouldBeHidden);

    if (bDisableTickWhenHidden)
        Owner->SetActorTickEnabled(!bShouldBeHidden);

    bWasHidden = bShouldBeHidden;
}
