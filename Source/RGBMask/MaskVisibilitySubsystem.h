#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MaskTypes.h"
#include "MaskVisibilitySubsystem.generated.h"

class UMaskVisibilityComponent;
class ARGBMaskCharacter;

UCLASS()
class UMaskVisibilitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    void Register(UMaskVisibilityComponent* Comp);
    void Unregister(UMaskVisibilityComponent* Comp);

    EMaskType GetCurrentMask() const { return CurrentMask; }   

private:
    void EnsureBoundToPlayer();
    UFUNCTION()
    void OnPlayerMaskChanged(EMaskType NewMask);

    void ApplyMaskToAll(bool bAllowFX);
    void PruneInvalid();

    void ScheduleBindRetry();
private:
    EMaskType CurrentMask = EMaskType::None;

    TSet<TWeakObjectPtr<UMaskVisibilityComponent>> Registered;

    TWeakObjectPtr<ARGBMaskCharacter> CachedPlayer;
    bool bBoundToPlayer = false;
    FTimerHandle RetryBindHandle;

};


