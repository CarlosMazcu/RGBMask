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

FLinearColor UMaskVisibilityComponent:: GetFXColorForMask(EMaskType Mask) const
{
    switch (Mask)
    {
    case EMaskType::Red:   return RedFXColor;
    case EMaskType::Green: return GreenFXColor;
    case EMaskType::Blue:  return BlueFXColor;
    default:               return FLinearColor::White;
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



void UMaskVisibilityComponent::ApplyMask(EMaskType Mask, bool bAllowFX)
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
    if (!bAllowFX)
    {
        if (ActiveHideFXComponent)
        {
            ActiveHideFXComponent->DeactivateImmediate();
            ActiveHideFXComponent->DestroyComponent();
            ActiveHideFXComponent = nullptr;
        }
    }
    else if (bChangingState)
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
                    false,
                    ENCPoolMethod::None,
                    true
                );
                if (ActiveHideFXComponent)
                {
                    const FLinearColor FXColor = GetFXColorForMask(Mask);

                    ActiveHideFXComponent->SetVariableLinearColor(MaskColorParamName, FXColor);
                    ActiveHideFXComponent->Activate(true);
                }
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
