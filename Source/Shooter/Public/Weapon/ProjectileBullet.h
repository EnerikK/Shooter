// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile.h"
#include "ProjectileBullet.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AProjectileBullet : public AProjectile
{
	GENERATED_BODY()

public:

	AProjectileBullet();
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
protected:

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void BeginPlay() override;
};
