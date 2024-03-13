// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Weapon.h"
#include "Flag.generated.h"

class UCombatComponent;
/**
 * 
 */
UCLASS()
class SHOOTER_API AFlag : public AWeapon
{
	GENERATED_BODY()

public:

	AFlag();
	virtual void Dropped() override;

protected:
	
	virtual void OnEquipped() override;
	virtual void OnDropped() override;

private:

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* FlagMesh;
	
};
