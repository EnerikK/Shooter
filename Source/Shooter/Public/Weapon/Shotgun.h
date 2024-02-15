// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "Weapon/HitScanWeapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;
	void ShotgunTraceEndWithScatter(const FVector& HitTarget,TArray<FVector>& HitTargets);

private:

	UPROPERTY(EditAnywhere,Category="Weapon Scatter")
	uint32 NumberOfPellets = 10;
	
};
