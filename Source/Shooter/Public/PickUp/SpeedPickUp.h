// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "PickUp/PickUp.h"
#include "SpeedPickUp.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API ASpeedPickUp : public APickUps
{
	GENERATED_BODY()

public:
	

protected:

	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:

	UPROPERTY(EditAnywhere)
	float BaseSpeedBuff = 1600.f;

	UPROPERTY(EditAnywhere)
	float CrouchSpeedBuff = 800.f;

	UPROPERTY(EditAnywhere)
	float SpeedBuffTime = 30.f;
};
