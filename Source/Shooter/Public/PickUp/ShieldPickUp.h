// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "PickUp/PickUp.h"
#include "ShieldPickUp.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AShieldPickUp : public APickUps
{
	GENERATED_BODY()

protected:

	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	

private:

	UPROPERTY(EditAnywhere)
	float ShieldReplenish = 20.f;
	
	UPROPERTY(EditAnywhere)
	float ShieldReplenishDuration = 4.f;
};
