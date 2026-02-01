#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "RGBMaskGameInstance.generated.h"

UCLASS()
class RGBMASK_API URGBMaskGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	/** Implement this in BP_GameInstance (no params) */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Save")
	void LoadGame();
};
