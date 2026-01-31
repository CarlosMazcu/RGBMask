// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "CameraVolume.h"
#include "RGBMaskCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class RGBMASK_API ARGBMaskCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	
public:
    ARGBMaskCameraManager();

    virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;

protected:
    UPROPERTY()
    ACameraVolume* ActiveCameraVolume;

    UPROPERTY()
    TArray<ACameraVolume*> CameraVolumes;

    // Buscar volúmenes en el nivel
    void FindCameraVolumes();

    // Determinar qué volumen usar basado en la posición del jugador
    ACameraVolume* GetActiveVolume(const FVector& PlayerLocation);

    // Aplicar el clamp a la cámara
    void ClampCameraToVolume(FVector& CameraLocation);

private:
    bool bVolumesInitialized = false;
};
