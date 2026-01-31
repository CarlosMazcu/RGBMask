// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/RGBMaskCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

ARGBMaskCameraManager::ARGBMaskCameraManager()
{
}

void ARGBMaskCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	// Inicializar volúmenes si es necesario
	if (!bVolumesInitialized)
	{
		FindCameraVolumes();
		bVolumesInitialized = true;
	}

	// Obtener pawn
	APlayerController* PC = GetOwningPlayerController();
	APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;
	if (!PlayerPawn)
	{
		// fallback: comportamiento base
		Super::UpdateViewTarget(OutVT, DeltaTime);
		return;
	}

	const FVector PlayerLocation = PlayerPawn->GetActorLocation();

	// Determinar volumen activo
	ActiveCameraVolume = GetActiveVolume(PlayerLocation);

	// 1) Base SIN modifiers (muy importante)
	Super::UpdateViewTargetInternal(OutVT, DeltaTime);

	// 2) Tu cámara (esto crea la POV “base”)
	ApplyCameraOffset(OutVT, PlayerLocation);

	// 3) Clamp (recomendado con margen para que el shake no se lo coma)
	if (ActiveCameraVolume)
	{
		constexpr float Slack = 25.f; // ajusta 10–50 según lo que quieras permitir
		FVector MinBounds, MaxBounds;
		ActiveCameraVolume->GetCameraBounds(MinBounds, MaxBounds);

		OutVT.POV.Location.X = FMath::Clamp(OutVT.POV.Location.X, MinBounds.X + Slack, MaxBounds.X - Slack);
		OutVT.POV.Location.Y = FMath::Clamp(OutVT.POV.Location.Y, MinBounds.Y + Slack, MaxBounds.Y - Slack);
	}

	// 4) Ahora sí: aplica modifiers (shakes, etc.)
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
	ActiveCameraVolume->GetCameraBounds(MinBounds, MaxBounds);

	// Clampear X e Y, mantener Z para altura de cámara
	CameraLocation.X = FMath::Clamp(CameraLocation.X, MinBounds.X, MaxBounds.X);
	CameraLocation.Y = FMath::Clamp(CameraLocation.Y, MinBounds.Y, MaxBounds.Y);
	// Z no se clampea para mantener la altura original
}

void ARGBMaskCameraManager::ApplyCameraOffset(FTViewTarget& OutVT, const FVector& PlayerLocation)
{
	APawn* PlayerPawn = GetOwningPlayerController()->GetPawn();
	if (!PlayerPawn)
		return;

	// Determinar la dirección hacia atrás
	FVector BackwardDirection;

	if (bUsePlayerForwardForOffset)
	{
		// Usar el forward del jugador (la cámara seguirá la orientación del personaje)
		FRotator PlayerRotation = PlayerPawn->GetActorRotation();
		BackwardDirection = -PlayerRotation.Vector(); // Negativo porque queremos ir hacia atrás
	}
	else
	{
		// Usar una dirección fija (por ejemplo, siempre hacia el sur)
		BackwardDirection = FVector(-1.0f, 0.0f, 0.0f); // Ajusta según tu nivel
	}

	// Proyectar en el plano horizontal (ignorar Z)
	BackwardDirection.Z = 0.0f;
	BackwardDirection.Normalize();

	// Calcular la posición objetivo de la cámara
	FVector TargetCameraLocation = PlayerLocation;

	// Aplicar offset horizontal (hacia atrás)
	TargetCameraLocation += BackwardDirection * CameraBackwardOffset;

	// Aplicar offset vertical (altura)
	TargetCameraLocation.Z += CameraHeightOffset;

	// Actualizar la posición de la cámara
	OutVT.POV.Location = TargetCameraLocation;

	// Calcular la rotación de la cámara para que mire al jugador
	FVector DirectionToPlayer = PlayerLocation - TargetCameraLocation;
	FRotator CameraRotation = DirectionToPlayer.Rotation();

	// Aplicar el pitch angle personalizado si se especifica
	if (FMath::Abs(CameraPitchAngle) > 0.01f)
	{
		CameraRotation.Pitch = CameraPitchAngle;
	}

	OutVT.POV.Rotation = CameraRotation;
}

