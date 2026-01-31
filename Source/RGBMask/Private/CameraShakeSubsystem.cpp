#include "CameraShakeSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "RGBMaskPlayerController.h"
#include "Camera/CameraShakeBase.h"

UCameraShakeSubsystem::UCameraShakeSubsystem()
{
    static ConstructorHelpers::FClassFinder<UCameraShakeBase> ShakeBP(
        TEXT("/Game/Actors/Traps/BP_ShakeBase.BP_ShakeBase_C")
    );

    if (ShakeBP.Succeeded())
    {
        DefaultImpactShake = ShakeBP.Class;
    }
}

void UCameraShakeSubsystem::PlayShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale, APlayerController* SpecificPC)
{
    if (!ShakeClass) return;
    UWorld* World = GetWorld();
    if (!World) return;

    // 1) Si te pasan un PC concreto, úsalo
    if (SpecificPC)
    {
        if (SpecificPC->IsLocalController() && SpecificPC->PlayerCameraManager)
        {
            SpecificPC->PlayerCameraManager->StartCameraShake(ShakeClass, Scale);
        }
        return;
    }
    APlayerController* PC0 = UGameplayStatics::GetPlayerController(World, 0);
    // 2) Si no, en singleplayer: PlayerController 0
    UCameraShakeBase* Started = PC0->PlayerCameraManager->StartCameraShake(ShakeClass, Scale);
    UE_LOG(LogTemp, Warning, TEXT("Shake started? %s"), Started ? TEXT("YES") : TEXT("NO"));

    if (Started)
    {
        if (PC0->IsLocalController() && PC0->PlayerCameraManager)
        {
            PC0->PlayerCameraManager->StartCameraShake(ShakeClass, Scale);
        }
    }
}

void UCameraShakeSubsystem::PlayImpactShake(float Scale, APlayerController* SpecificPC)
{
    PlayShake(DefaultImpactShake, Scale, SpecificPC);
}

void UCameraShakeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);


}

