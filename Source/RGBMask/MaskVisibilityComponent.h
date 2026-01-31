#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MaskTypes.h"
#include "MaskVisibilityComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UMaskVisibilityComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Mask")
    TArray<EMaskType> HiddenInMask;

    UPROPERTY(EditAnywhere, Category = "Mask")
    bool bDisableCollisionWhenHidden = true;

    UPROPERTY(EditAnywhere, Category = "Mask")
    bool bDisableTickWhenHidden = false;

    void ApplyMask(EMaskType Mask);

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
