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

	void GetVolumeBounds(FVector& OutMin, FVector& OutMax) const;

	bool IsLocationInsideVolume(const FVector& Location) const;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
