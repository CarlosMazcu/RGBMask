#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CameraShakeSubsystem.generated.h"

class UCameraShakeBase;

UCLASS()
class RGBMASK_API UCameraShakeSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCameraShakeSubsystem();

    /**
     * Plays a global camera shake (PlayerController 0 by default).
     *
     * If distance falloff is enabled (OuterRadius > InnerRadius and WorldSource is not ZeroVector),
     * the final intensity is scaled based on the camera distance to WorldSource:
     * - distance <= InnerRadius  => 100% intensity
     * - distance >= OuterRadius  => 0% intensity
     * - between                 => linearly interpolated
     *
     * @param Scale            Base intensity multiplier (1.0 = normal).
     * @param CooldownSeconds  Minimum time (seconds) between playing the SAME shake to prevent spam (0 = no cooldown).
     * @param WorldSource      World-space event origin (explosion/impact location). Used only for distance falloff.
     * @param InnerRadius      Inner falloff radius (cm). Inside this radius, intensity is 100%.
     * @param OuterRadius      Outer falloff radius (cm). Beyond this radius, intensity becomes 0.
     *                         If OuterRadius <= InnerRadius, falloff is disabled.
     * @param MinScale         Final scale clamp minimum (after falloff).
     * @param MaxScale         Final scale clamp maximum (after falloff).
     */
    UFUNCTION(BlueprintCallable, Category = "Camera|Shake",
        meta = (ToolTip = "Plays a global camera shake. Optional distance falloff using WorldSource + Inner/Outer radius."))
    void PlayShake(
        UPARAM(meta = (ToolTip = "Base intensity multiplier. 1.0 = normal.")) float Scale = 10.0f,

        UPARAM(meta = (ToolTip = "Cooldown (seconds) to prevent spam for the same shake. 0 = no cooldown.")) float CooldownSeconds = 0.12f,

        UPARAM(meta = (ToolTip = "World-space event origin (impact/explosion). Used only if falloff is enabled.")) FVector WorldSource = FVector::ZeroVector,

        UPARAM(meta = (ToolTip = "Inner radius (cm). At or below this distance, intensity is 100%.")) float InnerRadius = 0.0f,

        UPARAM(meta = (ToolTip = "Outer radius (cm). At or above this distance, intensity is 0. If Outer<=Inner, falloff is disabled.")) float OuterRadius = 0.0f,

        UPARAM(meta = (ToolTip = "Final intensity clamp minimum (applied after falloff).")) float MinScale = 0.0f,

        UPARAM(meta = (ToolTip = "Final intensity clamp maximum (applied after falloff).")) float MaxScale = 20.0f
    );


private:
    UPROPERTY(EditDefaultsOnly, Category = "Camera|Shake")
    TSubclassOf<UCameraShakeBase> DefaultImpactShake;
    TMap<TSubclassOf<UCameraShakeBase>, double> LastPlayTimeByShake;
};
