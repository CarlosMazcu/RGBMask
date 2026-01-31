#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MaskTypes.h"
#include "MaskVisibilityComponent.generated.h"

UENUM(BlueprintType)
enum class EMaskVisibilityMode : uint8
{
    HideInMasks   UMETA(DisplayName = "Hide In Masks", ToolTip = "Object is hidden when mask matches any in the list"),
    ShowOnlyInMasks UMETA(DisplayName = "Show Only In Masks", ToolTip = "Object is visible ONLY when mask matches any in the list")
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UMaskVisibilityComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Mask")
    TArray<EMaskType> HiddenInMask;

    UPROPERTY(EditAnywhere, Category = "Mask")
    EMaskVisibilityMode VisibilityMode = EMaskVisibilityMode::HideInMasks;

    UPROPERTY(EditAnywhere, Category = "Mask", meta = (EditCondition = "VisibilityMode == EMaskVisibilityMode::ShowOnlyInMasks", EditConditionHides))
    TArray<EMaskType> VisibleInMask;


    UPROPERTY(EditAnywhere, Category = "Mask")
    bool bDisableCollisionWhenHidden = true;

    UPROPERTY(EditAnywhere, Category = "Mask")
    bool bDisableTickWhenHidden = false;

    void ApplyMask(EMaskType Mask);


protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
