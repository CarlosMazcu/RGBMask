#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectilPoolComponent.generated.h"

class AActor;
class UProjectileMovementComponent;

USTRUCT(BlueprintType)
struct FProjectileSpawnParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform SpawnTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector InitialVelocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LifeTime = 0.0f; // 0 = no auto-release
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RGBMASK_API UProjectilPoolComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UProjectilPoolComponent();

	// Config
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pool")
	TSubclassOf<AActor> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pool", meta = (ClampMin = "1"))
	int32 InitialSize = 32;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pool", meta = (ClampMin = "0"))
	int32 ExpandBy = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pool")
	bool bAllowExpand = true;

	// API (Blueprint-friendly)
	UFUNCTION(BlueprintCallable, Category = "Pool")
	AActor* AcquireProjectile(const FProjectileSpawnParams& Params);

	UFUNCTION(BlueprintCallable, Category = "Pool")
	void ReleaseProjectile(AActor* Projectile);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> Available;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> InUse;

	void Prewarm(int32 Count);
	AActor* SpawnOne();

	void DeactivateProjectile(AActor* Projectile);
	void ActivateProjectile(AActor* Projectile, const FProjectileSpawnParams& Params);

	TMap<TObjectPtr<AActor>, FTimerHandle> LifeTimers;

};
