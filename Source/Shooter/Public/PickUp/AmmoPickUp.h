// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "PickUp/PickUp.h"
#include "Weapon/WeaponTypes.h"
#include "AmmoPickUp.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AAmmoPickUp : public APickUps
{
	GENERATED_BODY()

protected:

	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:

	UPROPERTY(EditAnywhere)
	int32 AmmoAmount = 30;
	
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;
	
};
