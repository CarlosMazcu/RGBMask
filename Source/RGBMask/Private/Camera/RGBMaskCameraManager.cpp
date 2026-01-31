// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/RGBMaskCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

ARGBMaskCameraManager::ARGBMaskCameraManager()
{
    PreviousCameraVolume = nullptr;
    ActiveCameraVolume = nullptr;
    CurrentCameraLocation = FVector::ZeroVector;
    CurrentCameraRotation = FRotator::ZeroRotator;
}

void ARGBMaskCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
    // Inicializar volumenes si es necesario
    if (!bVolumesInitialized)
    {
        FindCameraVolumes();
        bVolumesInitialized = true;
        UE_LOG(LogTemp, Warning, TEXT("[CameraManager] Camera volumes initialized, found %d volumes"), CameraVolumes.Num());
    }

    // Obtener pawn
    APlayerController* PC = GetOwningPlayerController();
    APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;
    if (!PlayerPawn)
    {
        Super::UpdateViewTarget(OutVT, DeltaTime);
        return;
    }

    const FVector PlayerLocation = PlayerPawn->GetActorLocation();

    // Determinar volumen activo
    ACameraVolume* NewActiveVolume = GetActiveVolume(PlayerLocation);

    // Detectar cambio de volumen
    if (NewActiveVolume != ActiveCameraVolume)
    {
        // IMPORTANTE: Iniciar transición ANTES de cambiar el volumen
        bIsTransitioning = true;

        UE_LOG(LogTemp, Log, TEXT("[CameraManager] Starting transition from volume %s to %s"),
            ActiveCameraVolume ? *ActiveCameraVolume->GetName() : TEXT("None"),
            NewActiveVolume ? *NewActiveVolume->GetName() : TEXT("None"));

        HandleVolumeChange(NewActiveVolume);
        ActiveCameraVolume = NewActiveVolume;
    }

    // 1) Base SIN modifiers
    Super::UpdateViewTargetInternal(OutVT, DeltaTime);

    // 2) Calcular la posición OBJETIVO de la cámara (donde queremos estar)
    FTViewTarget TempVT = OutVT; // Crear una copia temporal
    ApplyCameraOffset(TempVT, PlayerLocation);

    FVector TargetLocation = TempVT.POV.Location;
    FRotator TargetRotation = TempVT.POV.Rotation;

    // 3) Aplicar clamp al objetivo
    if (ActiveCameraVolume)
    {
        constexpr float Slack = 25.f;
        FVector MinBounds, MaxBounds;
        ActiveCameraVolume->GetCameraBounds(MinBounds, MaxBounds);

        TargetLocation.X = FMath::Clamp(TargetLocation.X, MinBounds.X + Slack, MaxBounds.X - Slack);
        TargetLocation.Y = FMath::Clamp(TargetLocation.Y, MinBounds.Y + Slack, MaxBounds.Y - Slack);
    }

    // 4) Interpolar hacia el objetivo
    if (bIsTransitioning)
    {
        // Interpolar desde la última posición conocida
        CurrentCameraLocation = FMath::VInterpTo(
            CurrentCameraLocation,
            TargetLocation,
            DeltaTime,
            CameraTransitionSpeed
        );

        if (bInterpolateRotation)
        {
            CurrentCameraRotation = FMath::RInterpTo(
                CurrentCameraRotation,
                TargetRotation,
                DeltaTime,
                CameraTransitionSpeed
            );
        }
        else
        {
            CurrentCameraRotation = TargetRotation;
        }

        // Comprobar si hemos llegado al objetivo
        float DistanceToTarget = FVector::Dist(CurrentCameraLocation, TargetLocation);
        if (DistanceToTarget < 1.0f)
        {
            bIsTransitioning = false;
            CurrentCameraLocation = TargetLocation;
            CurrentCameraRotation = TargetRotation;
            UE_LOG(LogTemp, Log, TEXT("[CameraManager] Transition completed"));
        }
    }
    else
    {
        // No hay transición: usar el objetivo directamente pero con interpolación suave
        CurrentCameraLocation = FMath::VInterpTo(
            CurrentCameraLocation,
            TargetLocation,
            DeltaTime,
            CameraTransitionSpeed * 2.0f // Más rápido cuando no hay cambio de volumen
        );
        CurrentCameraRotation = TargetRotation;
    }

    // 5) Aplicar la posición interpolada al viewport
    OutVT.POV.Location = CurrentCameraLocation;
    OutVT.POV.Rotation = CurrentCameraRotation;

    // 6) Aplicar modifiers (shakes, etc.)
    ApplyCameraModifiers(DeltaTime, OutVT.POV);
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

	// Si el jugador no esta en ningun volumen, usar el primero disponible
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
	ActiveCameraVolume->GetCameraBounds(MinBounds, MaxBounds);

	// Clampear X e Y, mantener Z para altura de camara
	CameraLocation.X = FMath::Clamp(CameraLocation.X, MinBounds.X, MaxBounds.X);
	CameraLocation.Y = FMath::Clamp(CameraLocation.Y, MinBounds.Y, MaxBounds.Y);
	// Z no se clampea para mantener la altura original
}

void ARGBMaskCameraManager::ApplyCameraOffset(FTViewTarget& OutVT, const FVector& PlayerLocation)
{
	APawn* PlayerPawn = GetOwningPlayerController()->GetPawn();
	if (!PlayerPawn)
		return;

	// Determinar la direccion hacia atras
	FVector BackwardDirection;

	if (bUsePlayerForwardForOffset)
	{
		// Usar el forward del jugador (la camara seguira la orientacion del personaje)
		FRotator PlayerRotation = PlayerPawn->GetActorRotation();
		BackwardDirection = -PlayerRotation.Vector(); // Negativo porque queremos ir hacia atras
	}
	else
	{
		// Usar una direccion fija (por ejemplo, siempre hacia el sur)
		BackwardDirection = FVector(-1.0f, 0.0f, 0.0f); // Ajusta segun tu nivel
	}

	// Proyectar en el plano horizontal (ignorar Z)
	BackwardDirection.Z = 0.0f;
	BackwardDirection.Normalize();

	// Calcular la posicion objetivo de la camara
	FVector TargetCameraLocation = PlayerLocation;

	// Aplicar offset horizontal (hacia atras)
	TargetCameraLocation += BackwardDirection * CameraBackwardOffset;

	// Aplicar offset vertical (altura)
	TargetCameraLocation.Z += CameraHeightOffset;

	// Actualizar la posicion de la camara
	OutVT.POV.Location = TargetCameraLocation;

	// Calcular la rotacion de la camara para que mire al jugador
	FVector DirectionToPlayer = PlayerLocation - TargetCameraLocation;
	FRotator CameraRotation = DirectionToPlayer.Rotation();

	// Aplicar el pitch angle personalizado si se especifica
	if (FMath::Abs(CameraPitchAngle) > 0.01f)
	{
		CameraRotation.Pitch = CameraPitchAngle;
	}

	OutVT.POV.Rotation = CameraRotation;
}

void ARGBMaskCameraManager::HandleVolumeChange(ACameraVolume* NewVolume)
{

	// Restaurar visibilidad del volumen anterior
	if (PreviousCameraVolume && PreviousCameraVolume != NewVolume)
	{
		PreviousCameraVolume->ShowActors();
	}

	// Ocultar actores del nuevo volumen
	if (NewVolume)
	{
		NewVolume->HideActors();
	}

	// Actualizar el volumen anterior
	PreviousCameraVolume = NewVolume;
}