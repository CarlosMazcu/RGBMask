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
    UFUNCTION(BlueprintCallable, Category = "Camera|Shake")
    void PlayShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale = 1.0f, APlayerController* SpecificPC = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Camera|Shake")
    void PlayImpactShake(float Scale = 1.0f, APlayerController* SpecificPC = nullptr);

    void Initialize(FSubsystemCollectionBase& Collection);

private:
    UPROPERTY(EditDefaultsOnly, Category = "Camera|Shake")
    TSubclassOf<UCameraShakeBase> DefaultImpactShake;
};
