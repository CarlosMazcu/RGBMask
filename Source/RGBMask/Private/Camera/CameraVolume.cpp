// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/CameraVolume.h"
#include "Components/BrushComponent.h"

// Sets default values
ACameraVolume::ACameraVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    
    if (GetBrushComponent())
    {
        GetBrushComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        GetBrushComponent()->SetMobility(EComponentMobility::Static);
    }

    // Hacer el volume visible en el editor
    bColored = true;
    BrushColor = FColor(255, 200, 100, 255);
}

void ACameraVolume::GetVolumeBounds(FVector& OutMin, FVector& OutMax) const
{
    FVector Origin, BoxExtent;
    GetActorBounds(false, Origin, BoxExtent);

    OutMin = Origin - BoxExtent;
    OutMax = Origin + BoxExtent;
}

bool ACameraVolume::IsLocationInsideVolume(const FVector& Location) const
{
    FVector MinBounds, MaxBounds;
    GetVolumeBounds(MinBounds, MaxBounds);

    // Crear un FBox y verificar si el punto está dentro
    FBox VolumeBox(MinBounds, MaxBounds);
    return VolumeBox.IsInside(Location);
}

void ACameraVolume::GetCameraBounds(FVector& OutMin, FVector& OutMax) const
{
    // Obtener los límites base del volumen
    GetVolumeBounds(OutMin, OutMax);

    // Expandir los límites con el margen de cámara
    OutMin -= CameraMargin;
    OutMax += CameraMargin;
}

// Called when the game starts or when spawned
void ACameraVolume::BeginPlay()
{
	Super::BeginPlay();
	
    UE_LOG(LogTemp, Warning, TEXT("[CameraVolume] BeginPlay - Volume: %s, HiddenActors count: %d"), *GetName(), HiddenActors.Num());

    for (AActor* Actor : HiddenActors)
    {
        if (IsValid(Actor))
        {
            OriginalVisibilityState.Add(Actor, Actor->IsHidden());
            UE_LOG(LogTemp, Log, TEXT("[CameraVolume] Registered actor: %s (originally hidden: %s)"),
                *Actor->GetName(),
                Actor->IsHidden() ? TEXT("YES") : TEXT("NO"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[CameraVolume] Invalid actor in HiddenActors list at BeginPlay!"));
        }
    }

    
}

// Called every frame
void ACameraVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACameraVolume::HideActors()
{
    UE_LOG(LogTemp, Warning, TEXT("[CameraVolume] HideActors called on %s, HiddenActors count: %d"), *GetName(), HiddenActors.Num());

    for (AActor* Actor : HiddenActors)
    {
        if (IsValid(Actor))
        {
            // Guardar el estado original de visibilidad si no lo hemos guardado antes
            if (!OriginalVisibilityState.Contains(Actor))
            {
                OriginalVisibilityState.Add(Actor, Actor->IsHidden());
            }

            UE_LOG(LogTemp, Warning, TEXT("[CameraVolume] Hiding actor: %s (was hidden: %s)"),
                *Actor->GetName(),
                Actor->IsHidden() ? TEXT("YES") : TEXT("NO"));

            // Ocultar el actor
            Actor->SetActorHiddenInGame(true);

            UE_LOG(LogTemp, Warning, TEXT("[CameraVolume] After hiding, actor %s is hidden: %s"),
                *Actor->GetName(),
                Actor->IsHidden() ? TEXT("YES") : TEXT("NO"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[CameraVolume] Invalid actor in HiddenActors list!"));
        }
    }
}

void ACameraVolume::ShowActors()
{
    UE_LOG(LogTemp, Warning, TEXT("[CameraVolume] ShowActors called on %s"), *GetName());

    for (AActor* Actor : HiddenActors)
    {
        if (IsValid(Actor))
        {
            // Restaurar el estado original de visibilidad
            bool bWasOriginallyHidden = false;
            if (OriginalVisibilityState.Contains(Actor))
            {
                bWasOriginallyHidden = OriginalVisibilityState[Actor];
            }

            UE_LOG(LogTemp, Warning, TEXT("[CameraVolume] Showing actor: %s (restore to originally hidden: %s)"),
                *Actor->GetName(),
                bWasOriginallyHidden ? TEXT("YES") : TEXT("NO"));

            // Solo mostrar si no estaba oculto originalmente
            Actor->SetActorHiddenInGame(bWasOriginallyHidden);
        }
    }
}