// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "PickUp/PickUp.h"
#include "HealthPickUp.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
/**
 * 
 */
UCLASS()
class SHOOTER_API AHealthPickUp : public APickUps
{
	GENERATED_BODY()

public:

	AHealthPickUp();
	
protected:

	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;


private:

	UPROPERTY(EditAnywhere)
	float HealAmount = 20.f;
	
	UPROPERTY(EditAnywhere)
	float HealingTime = 4.f;
	
};
