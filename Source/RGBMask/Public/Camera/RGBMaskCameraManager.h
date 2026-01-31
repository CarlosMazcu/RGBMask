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
    ACameraVolume* PreviousCameraVolume;

    UPROPERTY()
    TArray<ACameraVolume*> CameraVolumes;

    // Camera offset settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Offset", meta = (ToolTip = "Distancia hacia atrás desde el jugador"))
    float CameraBackwardOffset = 300.0f; 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Offset", meta = (ToolTip = "Altura sobre el jugador"))
    float CameraHeightOffset = 400.0f;  

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Offset", meta = (ToolTip = "Ángulo de inclinación de la camara"))
    float CameraPitchAngle = -45.0f; 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Offset", meta = (ToolTip = "Si true, usa el forward del jugador; si false, usa dirección fija"))
    bool bUsePlayerForwardForOffset = true;  


    // Transition settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Transition", meta = (ToolTip = "Velocidad de interpolación de la cámara entre volúmenes"))
    float CameraTransitionSpeed = 5.0f; // Ajusta según necesites (más alto = más rápido)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Transition", meta = (ToolTip = "Si true, interpola también la rotación de la cámara"))
    bool bInterpolateRotation = true;

    // Buscar volúmenes en el nivel
    void FindCameraVolumes();

    // Determinar qué volumen usar basado en la posición del jugador
    ACameraVolume* GetActiveVolume(const FVector& PlayerLocation);

    // Aplicar el clamp a la cámara
    void ClampCameraToVolume(FVector& CameraLocation);

    void ApplyCameraOffset(FTViewTarget& OutVT, const FVector& PlayerLocation);

    // Manejar el cambio de volumen activo
    void HandleVolumeChange(ACameraVolume* NewVolume);

private:
    bool bVolumesInitialized = false;

    // Transition state
    bool bIsTransitioning = false;
    FVector CurrentCameraLocation; 
    FRotator CurrentCameraRotation;
    FVector TransitionStartLocation;
    FRotator TransitionStartRotation;
    FVector TransitionTargetLocation;
    FRotator TransitionTargetRotation;
};
