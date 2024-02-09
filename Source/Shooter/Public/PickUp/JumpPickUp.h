// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "PickUp/PickUp.h"
#include "JumpPickUp.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AJumpPickUp : public APickUps
{
	GENERATED_BODY()

protected:

	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;


private:

	UPROPERTY(EditAnywhere)
	float JumpZVelocityBuff = 4000.f;

	UPROPERTY(EditAnywhere)
	float JumpBuffDuration = 10.f;
	
};
