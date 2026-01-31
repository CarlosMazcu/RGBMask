// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/RGBMaskCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

ARGBMaskCameraManager::ARGBMaskCameraManager()
{
}

void ARGBMaskCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
    // Llamar a la implementación base primero
    Super::UpdateViewTarget(OutVT, DeltaTime);

    // Inicializar volúmenes si es necesario
    if (!bVolumesInitialized)
    {
        FindCameraVolumes();
        bVolumesInitialized = true;
    }

    // Obtener la ubicación del jugador
    APawn* PlayerPawn = GetOwningPlayerController()->GetPawn();
    if (!PlayerPawn)
        return;

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    // Determinar volumen activo
    ActiveCameraVolume = GetActiveVolume(PlayerLocation);

    // Aplicar clamp si hay un volumen activo
    if (ActiveCameraVolume)
    {
        ClampCameraToVolume(OutVT.POV.Location);
    }
}

void ARGBMaskCameraManager::FindCameraVolumes()
{
    CameraVolumes.Empty();

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACameraVolume::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        ACameraVolume* Volume = Cast<ACameraVolume>(Actor);
        if (Volume)
        {
            CameraVolumes.Add(Volume);
        }
    }

    // Ordenar por prioridad (mayor prioridad primero)
    CameraVolumes.Sort([](const ACameraVolume& A, const ACameraVolume& B) {
        return A.Priority > B.Priority;
        });
}

ACameraVolume* ARGBMaskCameraManager::GetActiveVolume(const FVector& PlayerLocation)
{
    // Buscar el volumen de mayor prioridad que contenga al jugador
    for (ACameraVolume* Volume : CameraVolumes)
    {
        if (Volume && Volume->IsLocationInsideVolume(PlayerLocation))
        {
            return Volume;
        }
    }

    // Si el jugador no está en ningún volumen, usar el primero disponible
    if (CameraVolumes.Num() > 0)
    {
        return CameraVolumes[0];
    }

    return nullptr;
}

void ARGBMaskCameraManager::ClampCameraToVolume(FVector& CameraLocation)
{
    if (!ActiveCameraVolume)
        return;

    FVector MinBounds, MaxBounds;
    ActiveCameraVolume->GetVolumeBounds(MinBounds, MaxBounds);

    // Clampear X e Y, mantener Z para altura de cámara
    CameraLocation.X = FMath::Clamp(CameraLocation.X, MinBounds.X, MaxBounds.X);
    CameraLocation.Y = FMath::Clamp(CameraLocation.Y, MinBounds.Y, MaxBounds.Y);
    // Z no se clampea para mantener la altura original
}
