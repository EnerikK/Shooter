// Hello :) 


#include "PickUp/AmmoPickUp.h"
#include "Character/ShooterCharacter.h"
#include "Components/CombatComponent.h"
#include "Weapon/Weapon.h"

void AAmmoPickUp::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
	if(ShooterCharacter)
	{
		UCombatComponent* Combat = ShooterCharacter->GetCombat();
		if(Combat)
		{
			Combat->PickUpAmmo(WeaponType,AmmoAmount);
		}
	}
	Destroy();
}
