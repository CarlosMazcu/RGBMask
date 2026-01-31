// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MaskTypes.h"
#include "RGBMaskCharacter.generated.h"


class UCameraComponent;
class USpringArmComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMaskChanged, EMaskType);

/**
 *  A controllable top-down perspective character
 */
UCLASS(abstract)
class ARGBMaskCharacter : public ACharacter
{
	GENERATED_BODY()

private:

	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(EditAnywhere, Category = "Mask")
	EMaskType CurrentMask = EMaskType::None;

	TObjectPtr<UMaterialInterface> MainMaterial;
	int32 MaskMaterialIndex = 0;

	


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

	EMaskType GetMask() const { return CurrentMask; }
	void SetMask(EMaskType NewMask);
	
	UPROPERTY(EditDefaultsOnly, Category = "Mask|Materials")
	TObjectPtr<UMaterialInterface> RedMaskMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Mask|Materials")
	TObjectPtr<UMaterialInterface> GreenMaskMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Mask|Materials")
	TObjectPtr<UMaterialInterface> BlueMaskMaterial;

	FOnMaskChanged OnMaskChanged;

	TMap<EMaskType, bool> Masks;

	UFUNCTION(BlueprintCallable, Category = "Mask")

	void AddMaskToInventory(EMaskType mask);

	UFUNCTION(BlueprintCallable, Category = "Mask")
	void DeleteMask(EMaskType mask);

};

