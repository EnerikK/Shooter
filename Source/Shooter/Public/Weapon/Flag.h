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
	void ResetFlag();
	
	FORCEINLINE FTransform GetInitialTransform() const { return InitialTransform; }

protected:
	
	virtual void OnEquipped() override;
	virtual void OnDropped() override;
	virtual void BeginPlay() override;

private:
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* FlagMesh;

	FTransform InitialTransform;

	
};
