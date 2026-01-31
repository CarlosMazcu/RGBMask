#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "GameFramework/Actor.h"
#include "TrapSplineMover.generated.h"

class USplineComponent;
class UStaticMeshComponent;
class UBoxComponent;

UCLASS()
class ATrapSplineMover : public AActor
{
    GENERATED_BODY()

public:
    ATrapSplineMover();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<USplineComponent> Spline;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UStaticMeshComponent> TrapMesh;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UBoxComponent> StartTrigger;

    UPROPERTY(EditAnywhere, Category = "Stats")
    bool bStartOnTrigger = true;

    UPROPERTY(EditAnywhere, Category = "Stats")
    bool bLoop = false;

    UPROPERTY(EditAnywhere, Category = "Stats")
    bool bReverseAtEnd = false;

    UPROPERTY(EditAnywhere, Category = "Stats", meta = (ClampMin = "0.0"))
    float Speed = 600.f; // unidades UE por segundo

    UPROPERTY(EditAnywhere, Category = "Stats", meta = (ClampMin = "0.0"))
    float StartDistance = 0.f;

    UPROPERTY(EditAnywhere, Category = "Stats|Collision")
    bool bSweepCollision = true;

    UPROPERTY(EditAnywhere, Category = "Stats|Collision", meta = (EditCondition = "bSweepCollision", EditConditionHides))
    bool bStopOnHit = false;

private:
    bool bActive = false;
    float Distance = 0.f;
    int32 DirectionSign = +1; // +1 adelante, -1 atrás

    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    void SetTrapTransformAtDistance(float InDistance, float DeltaSeconds);


    UPROPERTY(EditAnywhere, Category = "Stats|Gamefeel")
    TSubclassOf<UCameraShakeBase> HitCameraShake;

    UPROPERTY(EditAnywhere, Category = "Stats|Gamefeel", meta = (ClampMin = "0.0"))
    float HitCameraShakeScale = 1.0f;

};
