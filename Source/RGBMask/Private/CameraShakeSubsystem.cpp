#include "CameraShakeSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "RGBMaskPlayerController.h"
#include "Camera/CameraShakeBase.h"

UCameraShakeSubsystem::UCameraShakeSubsystem()
{
    static ConstructorHelpers::FClassFinder<UCameraShakeBase> ShakeBP(
        TEXT("/Game/Actors/Traps/BP_ShakeBase.BP_ShakeBase_C")
    );

    if (ShakeBP.Succeeded())
    {
        DefaultImpactShake = ShakeBP.Class;
    }
}

void UCameraShakeSubsystem::PlayShake(
    float Scale,
    float CooldownSeconds,
    FVector WorldSource,
    float InnerRadius,
    float OuterRadius,
    float MinScale,
    float MaxScale,
    bool bPlayRumble,
    float RumbleIntensity,
    float RumbleDuration,
    float MinRumble,
    float MaxRumble
)
{
    UWorld* World = GetWorld();
    if (!World || !DefaultImpactShake) return;

    // Cooldown por shake (y de paso evita spamear la vibración también)
    const double Now = World->GetTimeSeconds();
    if (CooldownSeconds > 0.f)
    {
        const double* LastPtr = LastPlayTimeByShake.Find(DefaultImpactShake);
        if (LastPtr && (Now - *LastPtr) < CooldownSeconds)
        {
            return;
        }
    }

    APlayerController* PC0 = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC0 || !PC0->IsLocalController() || !PC0->PlayerCameraManager) return;

    float FinalScale = Scale;

    // Falloff por distancia si OuterRadius > InnerRadius y WorldSource válido
    const bool bUseFalloff =
        !WorldSource.IsNearlyZero() &&
        (OuterRadius > InnerRadius) &&
        (OuterRadius > 0.f);

    float Falloff = 1.f; // 1 => sin falloff
    if (bUseFalloff)
    {
        const FVector CamLoc = PC0->PlayerCameraManager->GetCameraLocation();
        const float Dist = FVector::Dist(CamLoc, WorldSource);

        // Dist <= inner => 1; Dist >= outer => 0
        const float Alpha = FMath::Clamp((Dist - InnerRadius) / (OuterRadius - InnerRadius), 0.f, 1.f);
        Falloff = 1.f - Alpha;

        FinalScale *= Falloff;
    }

    FinalScale = FMath::Clamp(FinalScale, MinScale, MaxScale);
    if (FinalScale <= KINDA_SMALL_NUMBER) return;

    // --- Camera shake ---
    PC0->PlayerCameraManager->StartCameraShake(DefaultImpactShake, FinalScale);

    // --- Gamepad rumble (a todos los motores por igual) ---
    // Intensidad 0..1. Si no hay mando/rumble, no pasa nada (simplemente no vibra).
    if (bPlayRumble && RumbleDuration > KINDA_SMALL_NUMBER)
    {
        float FinalRumble = RumbleIntensity;

        if (bUseFalloff)
        {
            FinalRumble *= Falloff;
        }

        FinalRumble = FMath::Clamp(FinalRumble, MinRumble, MaxRumble);

        if (FinalRumble > KINDA_SMALL_NUMBER)
        {
            FDynamicForceFeedbackHandle Handle{}; 
            PC0->PlayDynamicForceFeedback(
                FinalRumble,
                RumbleDuration,
                true,  // Left Large
                true,  // Left Small
                true,  // Right Large
                true,  // Right Small
                EDynamicForceFeedbackAction::Start,
                Handle
            );
        }
    }

    if (CooldownSeconds > 0.f)
    {
        LastPlayTimeByShake.Add(DefaultImpactShake, Now);
    }
}


