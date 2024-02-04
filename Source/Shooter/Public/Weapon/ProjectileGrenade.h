// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile.h"
#include "ProjectileGrenade.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AProjectileGrenade : public AProjectile
{
	GENERATED_BODY()

public:
	
	AProjectileGrenade();

protected:

	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult,const FVector& ImpactVelocity);

private:

	UPROPERTY(EditAnywhere)
	USoundCue* BounceSound; 
};
