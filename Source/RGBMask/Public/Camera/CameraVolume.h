// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "CameraVolume.generated.h"

UCLASS()
class RGBMASK_API ACameraVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACameraVolume();

	UPROPERTY(EditAnywhere, Category = "Camera")
	UBoxComponent* CameraVolumeBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	int32 Priority = 0; // posible mejora volumenes superpuestos

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ToolTip = "Margen adicional que permite a la cámara salirse del volumen"))
	FVector CameraMargin = FVector(200.0f, 200.0f, 0.0f);

	void GetVolumeBounds(FVector& OutMin, FVector& OutMax) const;

	bool IsLocationInsideVolume(const FVector& Location) const;

	void GetCameraBounds(FVector& OutMin, FVector& OutMax) const;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
