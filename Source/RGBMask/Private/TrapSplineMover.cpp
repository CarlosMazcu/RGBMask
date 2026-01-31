#include "TrapSplineMover.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

ATrapSplineMover::ATrapSplineMover()
{
    PrimaryActorTick.bCanEverTick = true;

    Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
    RootComponent = Spline;

    TrapMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrapMesh"));
    TrapMesh->SetupAttachment(RootComponent);
    TrapMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TrapMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    TrapMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);   // o Overlap
    TrapMesh->SetGenerateOverlapEvents(true);

    StartTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("StartTrigger"));
    StartTrigger->SetupAttachment(RootComponent);
    StartTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    StartTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    StartTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    StartTrigger->SetGenerateOverlapEvents(true);
}

void ATrapSplineMover::BeginPlay()
{
    Super::BeginPlay();

    Distance = StartDistance;
    SetTrapTransformAtDistance(Distance, 0.f);

    if (bStartOnTrigger)
    {
        StartTrigger->OnComponentBeginOverlap.AddDynamic(this, &ATrapSplineMover::OnTriggerBeginOverlap);
    }
    else
    {
        bActive = true;
    }
}

void ATrapSplineMover::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (Cast<ACharacter>(OtherActor))
    {
        bActive = true;
        UE_LOG(LogTemp, Warning, TEXT("Espabila que te cogen"));

        // desactivar trigger para que no re-dispare
        StartTrigger->SetGenerateOverlapEvents(false);
        StartTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void ATrapSplineMover::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!bActive || !Spline) return;

    const float SplineLen = Spline->GetSplineLength();
    if (SplineLen <= KINDA_SMALL_NUMBER) return;

    Distance += DirectionSign * Speed * DeltaSeconds;

    // Manejo fin de spline
    if (Distance >= SplineLen)
    {
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

    SetTrapTransformAtDistance(Distance, DeltaSeconds);
}

void ATrapSplineMover::SetTrapTransformAtDistance(float InDistance, float DeltaSeconds)
{
    const FVector NewLoc = Spline->GetLocationAtDistanceAlongSpline(InDistance, ESplineCoordinateSpace::World);
    const FRotator NewRot = Spline->GetRotationAtDistanceAlongSpline(InDistance, ESplineCoordinateSpace::World);

    FHitResult Hit;
    TrapMesh->SetWorldLocationAndRotation(NewLoc, NewRot, bSweepCollision, &Hit);

    if (Hit.bBlockingHit)
    {
        // Si quieres que mate al player:
        if (ACharacter* Char = Cast<ACharacter>(Hit.GetActor()))
        {
            if (HitCameraShake)
            {
                if (APlayerController* PC = Cast<APlayerController>(Char->GetController()))
                {
                    PC->ClientStartCameraShake(HitCameraShake, HitCameraShakeScale);
                }
                else
                {
                    // fallback: si no es el pawn del player, intenta con player0
                    if (APlayerController* PC0 = UGameplayStatics::GetPlayerController(GetWorld(), 0))
                    {
                        PC0->ClientStartCameraShake(HitCameraShake, HitCameraShakeScale);
                    }
                }
            }

            // TODO: aplicar daño / muerte / empuje
            UE_LOG(LogTemp, Warning, TEXT("Trampa golpeó a %s"), *Char->GetName());
        }

        if (bStopOnHit)
        {
            bActive = false;
        }
    }
}
