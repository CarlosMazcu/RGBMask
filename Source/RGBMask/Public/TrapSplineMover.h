#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrapSplineMover.generated.h"

class USplineComponent;
class UStaticMeshComponent;
class UBoxComponent;
class UPrimitiveComponent;

UCLASS()
class ATrapSplineMover : public AActor
{
    GENERATED_BODY()

public:
    ATrapSplineMover();

    UFUNCTION(BlueprintCallable, Category = "Reset")
    void ResetWallTrap();

    UFUNCTION(BlueprintCallable, Category = "Reset")
    void ScheduleResetWallTrap(float DelaySeconds);

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
    float Speed = 600.f;

    UPROPERTY(EditAnywhere, Category = "Stats", meta = (ClampMin = "0.0"))
    float StartDistance = 0.f;

    UPROPERTY(EditAnywhere, Category = "Stats|Collision", DisplayName = "Sweep (solo Pawn)")
    bool bSweepCollision = true;

    UPROPERTY(EditAnywhere, Category = "Stats|Collision", meta = (EditCondition = "bSweepCollision", EditConditionHides), DisplayName = "Stop On Hit (Pawn)")
    bool bStopOnHit = false;

    UPROPERTY(EditAnywhere, Category = "Stats|Collision", meta = (EditCondition = "bSweepCollision"), DisplayName = "Disable Mesh Collision On Pawn Hit")
    bool bDisableMeshCollisionOnPawnHit = true;

    UPROPERTY(EditAnywhere, Category = "Reset", meta = (ClampMin = "0.0"))
    float DefaultResetDelay = 1.5f;

private:

    float InitialDistance = 0.f;
    int32 InitialDirectionSign = +1;

    FTimerHandle ResetTimerHandle;

    void RestorePawnOnlyCollision();
    bool bActive = false;
    float Distance = 0.f;
    int32 DirectionSign = +1;

    bool bHasDisabledMeshCollision = false;

    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    void SetTrapTransformAtDistance(float InDistance, float DeltaSeconds);

    // --- Gamefeel: vibración del mesh al impactar ---
    UPROPERTY(EditAnywhere, Category = "Stats|Gamefeel", meta = (ClampMin = "0.0"))
    float MeshShakeDuration = 0.18f;

    UPROPERTY(EditAnywhere, Category = "Stats|Gamefeel", meta = (ClampMin = "0.0"))
    float MeshShakeStrength = 14.0f; // cm

    UPROPERTY(EditAnywhere, Category = "Stats|Gamefeel", meta = (ClampMin = "0.0"))
    float MeshShakeFrequency = 28.0f; // Hz aprox

    UPROPERTY(EditAnywhere, Category = "Stats|Gamefeel", meta = (ClampMin = "0.0"))
    float MeshShakeDecay = 10.0f; // mayor => se apaga más rápido

    UPROPERTY(EditAnywhere, Category = "Stats|Gamefeel")
    bool bShakeAlsoRotates = true;

    UPROPERTY(EditAnywhere, Category = "Stats|Gamefeel", meta = (ClampMin = "0.0"))
    float MeshShakeRotStrength = 2.0f; // grados

    float MeshShakeTimeLeft = 0.f;
    float MeshShakeSeed = 0.f;

    UPROPERTY(EditAnywhere, Category = "Stats|Gamefeel", meta = (ClampMin = "0.0"))
    float MeshShakePosStrength = 1.5f; 

    UPROPERTY(EditAnywhere, Category = "Stats|Gamefeel", meta = (ClampMin = "0.0"))
    float MeshShakeInterpSpeed = 80.0f;

    FVector SmoothedWorldOffset = FVector::ZeroVector;
    FRotator SmoothedRotOffset = FRotator::ZeroRotator;
};
