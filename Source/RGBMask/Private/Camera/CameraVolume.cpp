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
	
}

// Called every frame
void ACameraVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

