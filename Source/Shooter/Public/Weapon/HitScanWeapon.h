// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

public:

	virtual void Fire(const FVector& HitTarget) override;
	

protected:
	
	void WeaponTraceHit(const FVector& TraceStart,const FVector& HitTarget,FHitResult& OutHit);

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	USoundCue* HitSound;

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

private:
	
	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;
	
	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticle;
	
};
