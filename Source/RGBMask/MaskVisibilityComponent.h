#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MaskTypes.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
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
    UMaskVisibilityComponent();
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

private:
    UPROPERTY(VisibleDefaultsOnly, Category = "Mask|FX")
    TObjectPtr<class UNiagaraSystem> HideFX = nullptr;

    UPROPERTY(EditAnywhere, Category = "Mask|FX")
    FVector HideFXOffset = FVector::ZeroVector;

    UPROPERTY(Transient)
    TObjectPtr<UNiagaraComponent> ActiveHideFXComponent = nullptr;

    bool bWasHidden = false;

    UPROPERTY(EditDefaultsOnly, Category = "Mask|FX")
    FLinearColor RedFXColor = FLinearColor(1.0f, 0.35f, 0.35f, 1.0f);   // rojo más “vivo”

    UPROPERTY(EditDefaultsOnly, Category = "Mask|FX")
    FLinearColor GreenFXColor = FLinearColor(0.35f, 1.0f, 0.35f, 1.0f);   // verde más “vivo”

    UPROPERTY(EditDefaultsOnly, Category = "Mask|FX")
    FLinearColor BlueFXColor = FLinearColor(0.35f, 0.60f, 1.0f, 1.0f);   // azul más visible

    UPROPERTY(EditDefaultsOnly, Category = "Mask|FX")
    FName MaskColorParamName = TEXT("User.MaskColor");

    FLinearColor GetFXColorForMask(EMaskType Mask) const;

};
