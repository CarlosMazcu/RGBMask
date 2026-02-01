// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MaskTypes.h"
#include "RGBMaskCharacter.generated.h"


class UCameraComponent;
class USpringArmComponent;
class APostProcessVolume;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaskChanged, EMaskType, MaskType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaskChangeStarted, EMaskType, MaskType);

/**
 *  A controllable top-down perspective character
 */
UCLASS(abstract)
class ARGBMaskCharacter : public ACharacter
{
	GENERATED_BODY()

private:

	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(EditAnywhere, Category = "Mask")
	EMaskType CurrentMask = EMaskType::None;

	UPROPERTY(EditAnywhere, Category = "Mask")
	EMaskType PendingMask = EMaskType::None;

	UPROPERTY(EditAnywhere, Category = "Mask", meta = (ClampMin = "0.0"))
	float MaskChangeDelay = 0.5f;

	FTimerHandle MaskChangeTimerHandle;
	bool bIsMaskChangeInProgress = false;

	TObjectPtr<UMaterialInterface> MainMaterial;
	int32 MaskMaterialIndex = 0;

	/** Internal function that applies the mask change after delay */
	void ApplyMaskChange();

	void UpdatePostProcess();

	UPROPERTY(EditAnywhere, Category = "Mask|PostProcess|Advanced", meta = (ClampMin = "0.0"))
	float PostProcessBlendDuration = 0.3f;

	FTimerHandle PostProcessBlendTimerHandle;
	float PostProcessBlendAlpha = 0.0f;
	EMaskType PreviousMask = EMaskType::None;
	bool bIsBlendingPostProcess = false;

	void TickPostProcessBlend();
	void StartPostProcessBlend();

	UPROPERTY(EditAnywhere, Category = "Camera")
	float CameraDistance = 800.f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float CameraPitch = -60.f;


public:

	/** Constructor */
	ARGBMaskCharacter();

	/** Initialization */
	virtual void BeginPlay() override;

	/** Update */
	virtual void Tick(float DeltaSeconds) override;

	/** Returns the camera component **/
	UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent.Get(); }

	/** Returns the Camera Boom component **/
	USpringArmComponent* GetCameraBoom() const { return CameraBoom.Get(); }

	UFUNCTION(BlueprintCallable)
	EMaskType GetMask() const { return CurrentMask; }
	void SetMask(EMaskType NewMask);

	UPROPERTY(EditDefaultsOnly, Category = "Mask|Materials")
	TObjectPtr<UMaterialInterface> RedMaskMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Mask|Materials")
	TObjectPtr<UMaterialInterface> GreenMaskMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Mask|Materials")
	TObjectPtr<UMaterialInterface> BlueMaskMaterial;

	// ============================================
	// POST PROCESS SETTINGS
	// ============================================

	/**
	 * PostProcessVolume reference - will be auto-detected from the level.
	 * You can also manually assign it by selecting this character in the level (not in BP defaults!)
	 */
	UPROPERTY(VisibleInstanceOnly, Category = "Mask|PostProcess|Debug")
	TObjectPtr<APostProcessVolume> PostProcessVolume;

	UPROPERTY(EditDefaultsOnly, Category = "Mask|PostProcess")
	TObjectPtr<UMaterialInstance> RedPostProcessMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Mask|PostProcess")
	TObjectPtr<UMaterialInstance> GreenPostProcessMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Mask|PostProcess")
	TObjectPtr<UMaterialInstance> BluePostProcessMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Mask|PostProcess")
	TObjectPtr<UMaterialInstance> NonePostProcessMaterial;

	UPROPERTY(EditAnywhere, Category = "Mask|PostProcess")
	bool bUsePostProcessEffects = true;

	UPROPERTY(EditAnywhere, Category = "Mask|PostProcess", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PostProcessBlendWeight = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Mask|PostProcess|Advanced")
	bool bUseSmoothBlending = false;  // Changed to false by default for easier debugging

	UPROPERTY(BlueprintAssignable)
	FOnMaskChanged OnMaskChanged;

	UPROPERTY(BlueprintAssignable)
	FOnMaskChangeStarted OnMaskChangeStarted;

	TMap<EMaskType, bool> Masks;

	UFUNCTION(BlueprintCallable, Category = "Mask")

	void AddMaskToInventory(EMaskType mask);

	UFUNCTION(BlueprintCallable, Category = "Mask")
	void DeleteMask(EMaskType mask);

	// DEBUG FUNCTIONS
	UFUNCTION(BlueprintCallable, Category = "Mask|PostProcess|Debug")
	void DebugPrintPostProcessInfo();

	UFUNCTION(BlueprintCallable, Category = "Mask|PostProcess|Debug")
	void ForceUpdatePostProcess();


};