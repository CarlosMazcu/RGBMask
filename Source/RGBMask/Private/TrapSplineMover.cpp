#include "TrapSplineMover.h"
#include "TimerManager.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "RGBMaskCharacter.h"
#include "Components/BoxComponent.h"
#include "CameraShakeSubsystem.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"


ATrapSplineMover::ATrapSplineMover()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PostPhysics;

    Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
    RootComponent = Spline;

    TrapMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrapMesh"));
    TrapMesh->SetupAttachment(RootComponent);

    // Opción B: el mesh IGNORA el mundo y SOLO interactúa con Pawn.
    // Así, el sweep nunca se atasca por tocar suelo/paredes.
    TrapMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TrapMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    TrapMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    TrapMesh->SetGenerateOverlapEvents(false);
    TrapMesh->SetSimulatePhysics(false);
    TrapMesh->SetCanEverAffectNavigation(false);

    StartTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("StartTrigger"));
    StartTrigger->SetupAttachment(RootComponent);
    StartTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    StartTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    StartTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    StartTrigger->SetGenerateOverlapEvents(true);
}

void ATrapSplineMover::ResetWallTrap()
{
    bActive = !bStartOnTrigger; 
    Distance = InitialDistance;
    DirectionSign = InitialDirectionSign;

    RestorePawnOnlyCollision();

    MeshShakeTimeLeft = 0.f;
    SmoothedWorldOffset = FVector::ZeroVector;
    SmoothedRotOffset = FRotator::ZeroRotator;

    if (bStartOnTrigger && StartTrigger)
    {
        StartTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        StartTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
        StartTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
        StartTrigger->SetGenerateOverlapEvents(true);
    }

    if (Spline && TrapMesh)
    {
        const FVector Loc = Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
        const FRotator Rot = Spline->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

        TrapMesh->SetWorldLocationAndRotation(Loc, Rot, false);
    }
}

void ATrapSplineMover::ScheduleResetWallTrap(float DelaySeconds)
{
    if (!GetWorld()) return;

    GetWorld()->GetTimerManager().ClearTimer(ResetTimerHandle);
    GetWorld()->GetTimerManager().SetTimer(
        ResetTimerHandle,
        this,
        &ATrapSplineMover::ResetWallTrap,
        DelaySeconds,
        false
    );
}

void ATrapSplineMover::BeginPlay()
{
    Super::BeginPlay();

    // Seed distinta por instancia (para que no vibren todos igual)
    MeshShakeSeed = FMath::FRandRange(0.f, 1000.f);

    InitialDistance = StartDistance;
    InitialDirectionSign = +1; 

    Distance = StartDistance;
    DirectionSign = InitialDirectionSign;
    ResetWallTrap();

    if (bStartOnTrigger)
    {
        StartTrigger->OnComponentBeginOverlap.AddDynamic(this, &ATrapSplineMover::OnTriggerBeginOverlap);
    }
    else
    {
        bActive = true;
    }
}

void ATrapSplineMover::RestorePawnOnlyCollision()
{
    if (!TrapMesh) return;

    TrapMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TrapMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    TrapMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    TrapMesh->SetGenerateOverlapEvents(false);
    TrapMesh->SetSimulatePhysics(false);
    TrapMesh->SetCanEverAffectNavigation(false);

    bHasDisabledMeshCollision = false;
}

void ATrapSplineMover::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (Cast<ACharacter>(OtherActor))
    {
        bActive = true;
        if (UWorld* w = GetWorld())
        {
            if (UCameraShakeSubsystem* ShakeSub = w->GetSubsystem<UCameraShakeSubsystem>())
            {
                ShakeSub->PlayShake(15.0f, 0.3f, FVector::ZeroVector, 0.0f, 0.0f, 0.0f, 25.0f, true, 20.0f, 8.0f, 0.0f, 30.0f);
            }
        }
        // desactivar trigger para que no re-dispare
        StartTrigger->SetGenerateOverlapEvents(false);
        StartTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void ATrapSplineMover::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!bActive || !Spline || !TrapMesh) return;
    IsInTheEnd = false;

    const float SplineLen = Spline->GetSplineLength();
    if (SplineLen <= KINDA_SMALL_NUMBER) return;

    Distance += DirectionSign * Speed * DeltaSeconds;

    // Manejo fin de spline
    if (Distance >= SplineLen)
    {
        IsInTheEnd = true;
        if (bReverseAtEnd)
        {
            Distance = SplineLen;
            DirectionSign = -1;
        }
        else if (bLoop)
        {
            Distance = 0.f;
        }
        else
        {
            Distance = SplineLen;
            bActive = false;
        }
    }
    else if (Distance <= 0.f)
    {
        if (bReverseAtEnd)
        {
            Distance = 0.f;
            DirectionSign = +1;
        }
        else if (bLoop)
        {
            Distance = SplineLen;
        }
        else
        {
            Distance = 0.f;
            bActive = false;
        }
    }
    if (bActive) 
    {
        if (UWorld* w = GetWorld())
        {
            if (UCameraShakeSubsystem* ShakeSub = w->GetSubsystem<UCameraShakeSubsystem>())
            {
                ShakeSub->PlayShake(7.0f, 0.1f, FVector::ZeroVector, 0.0f, 0.0f, 0.0f, 5.0f, true, 7.0f, 0.1f, 0.0f, 10.0f);
            }
        }
    }
    SetTrapTransformAtDistance(Distance, DeltaSeconds);
}

void ATrapSplineMover::SetTrapTransformAtDistance(float InDistance, float DeltaSeconds)
{
    const FVector NewLoc = Spline->GetLocationAtDistanceAlongSpline(InDistance, ESplineCoordinateSpace::World);
    const FRotator NewRot = Spline->GetRotationAtDistanceAlongSpline(InDistance, ESplineCoordinateSpace::World);

    FHitResult Hit;
    TrapMesh->SetWorldLocationAndRotation(NewLoc, NewRot, bSweepCollision, &Hit);

    auto TryKill = [&](AActor* A)
        {
            if (!A) return;
            if (ARGBMaskCharacter* Char = Cast<ARGBMaskCharacter>(A))
            {
                MeshShakeTimeLeft = MeshShakeDuration;

                if (bDisableMeshCollisionOnPawnHit && !bHasDisabledMeshCollision)
                {
                    TrapMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                    bHasDisabledMeshCollision = true;
                }

                if (bStopOnHit)
                {
                    bActive = false;
                }

                ScheduleResetWallTrap(DefaultResetDelay);
                Char->Die();

                if (DeathSFX)
                {
                    UGameplayStatics::PlaySound2D(this, DeathSFX, 1.0f);
                }
            }
        };

    if (!bSweepCollision)
    {
        // si no haces sweep, detecta por overlap
        TArray<AActor*> Overlaps;
        TrapMesh->GetOverlappingActors(Overlaps, ARGBMaskCharacter::StaticClass());
        if (Overlaps.Num() > 0)
        {
            TryKill(Overlaps[0]);
        }
        return;
    }

    if (Hit.bBlockingHit || Hit.bStartPenetrating)
    {
        TryKill(Hit.GetActor());
        return;
    }

    TArray<AActor*> Overlaps;
    TrapMesh->GetOverlappingActors(Overlaps, ARGBMaskCharacter::StaticClass());
    if (Overlaps.Num() > 0)
    {
        TryKill(Overlaps[0]);
    }

   /* if (MeshShakeTimeLeft > 0.f && MeshShakeDuration > 0.f)
    {
        MeshShakeTimeLeft = FMath::Max(0.f, MeshShakeTimeLeft - DeltaSeconds);

        const float Elapsed = MeshShakeDuration - MeshShakeTimeLeft;
        const float Envelope = FMath::Exp(-MeshShakeDecay * Elapsed); 

        const float W = 2.f * PI * MeshShakeFrequency;
        const float S1 = FMath::Sin((Elapsed + MeshShakeSeed) * W);
        const float S2 = FMath::Sin((Elapsed + MeshShakeSeed) * W * 1.37f);

        const FVector BaseLoc = TrapMesh->GetComponentLocation();
        const FRotator BaseRot = TrapMesh->GetComponentRotation();

        // --- TARGET de vibración ---
        // 1) Posición: MUY pequeña y en 1 eje (Y local) para que no “salte”
        const float PosAmp = MeshShakePosStrength * Envelope;
        const FVector TargetLocalOffset(0.f, S1 * PosAmp, 0.f);
        const FVector TargetWorldOffset = BaseRot.RotateVector(TargetLocalOffset);

        // 2) Rotación: aquí está el “feeling” principal
        FRotator TargetRotOffset = FRotator::ZeroRotator;
        if (bShakeAlsoRotates)
        {
            const float RotAmp = MeshShakeRotStrength * Envelope;
            // Pitch/Roll suelen sentirse mejor que yaw en top-down
            TargetRotOffset = FRotator(S2 * RotAmp, 0.f, S1 * RotAmp);
        }

        // --- Suavizado para evitar teleports ---
        SmoothedWorldOffset = FMath::VInterpTo(SmoothedWorldOffset, TargetWorldOffset, DeltaSeconds, MeshShakeInterpSpeed);
        SmoothedRotOffset = FMath::RInterpTo(SmoothedRotOffset, TargetRotOffset, DeltaSeconds, MeshShakeInterpSpeed);

        // Aplicar sin sweep (solo visual)
        TrapMesh->SetWorldLocationAndRotation(BaseLoc + SmoothedWorldOffset, BaseRot + SmoothedRotOffset, false);
    }
    else
    {
        // Reset al terminar (evita que se quede “desplazado”)
        SmoothedWorldOffset = FVector::ZeroVector;
        SmoothedRotOffset = FRotator::ZeroRotator;
    }
    */
}
