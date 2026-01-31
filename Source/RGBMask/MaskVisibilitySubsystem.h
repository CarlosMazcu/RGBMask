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

private:
    void EnsureBoundToPlayer();
    void OnPlayerMaskChanged(EMaskType NewMask);

    void ApplyMaskToAll();
    void PruneInvalid();

private:
    EMaskType CurrentMask = EMaskType::Red;

    TSet<TWeakObjectPtr<UMaskVisibilityComponent>> Registered;

    TWeakObjectPtr<ARGBMaskCharacter> CachedPlayer;
    bool bBoundToPlayer = false;
    FTimerHandle RetryBindHandle;
    void ScheduleBindRetry();

};


