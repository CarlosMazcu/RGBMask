#include "MaskVisibilityComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "MaskVisibilitySubsystem.h"
#include "Engine/World.h"


UMaskVisibilityComponent::UMaskVisibilityComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false; 
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

void UMaskVisibilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bPersistentFXWhileHidden || !bFollowFXToOwnerWhileHidden)
        return;

    if (!bWasHidden || !ActiveHideFXComponent)
        return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    const FVector NewLoc = Owner->GetActorLocation() + HideFXOffset;
    const FRotator NewRot = Owner->GetActorRotation();

    ActiveHideFXComponent->SetWorldLocationAndRotation(NewLoc, NewRot, false, nullptr, ETeleportType::TeleportPhysics);
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

    const bool bChangingState = (bShouldBeHidden != bWasHidden);

    // --- 1) FX persistente mientras está oculto (ideal para projectiles + pooling) ---
    if (bPersistentFXWhileHidden)
    {
        if (bShouldBeHidden)
        {
            // Asegurar que exista el FX aunque NO haya transición (pooling)
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
                    false,   // bAutoDestroy
                    false,   // bAutoActivate
                    ENCPoolMethod::None,
                    true
                );
                if (ActiveHideFXComponent)
                {
                    ActiveHideFXComponent->SetVariableLinearColor(MaskColorParamName, GetFXColorForMask(Mask));
                    ActiveHideFXComponent->Activate(true);
                }
            }
            else if (ActiveHideFXComponent)
            {
                // si cambias de máscara estando oculto, actualiza color
                ActiveHideFXComponent->SetVariableLinearColor(MaskColorParamName, GetFXColorForMask(Mask));
            }

            // activar tick solo si hay que seguir al owner
            SetComponentTickEnabled(bFollowFXToOwnerWhileHidden);
        }
        else
        {
            // si vuelve a aparecer, mata el FX persistente
            if (ActiveHideFXComponent)
            {
                ActiveHideFXComponent->DeactivateImmediate();
                ActiveHideFXComponent->DestroyComponent();
                ActiveHideFXComponent = nullptr;
            }
            SetComponentTickEnabled(false);
        }
    }
    // --- 2) FX “solo en transición” (tu comportamiento anterior) ---
    else
    {
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
                        ActiveHideFXComponent->SetVariableLinearColor(MaskColorParamName, GetFXColorForMask(Mask));
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

        SetComponentTickEnabled(false); // no follow en modo transición
    }

    // --- Visibilidad “core” (no lo tocamos) ---
    Owner->SetActorHiddenInGame(bShouldBeHidden);

    if (bDisableCollisionWhenHidden)
        Owner->SetActorEnableCollision(!bShouldBeHidden);

    if (bDisableTickWhenHidden)
        Owner->SetActorTickEnabled(!bShouldBeHidden);

    bWasHidden = bShouldBeHidden;
}
