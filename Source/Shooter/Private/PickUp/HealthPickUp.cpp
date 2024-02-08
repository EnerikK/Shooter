// Hello :) 


#include "PickUp/HealthPickUp.h"
#include "Character/ShooterCharacter.h"
#include "Components/BuffComponent.h"

AHealthPickUp::AHealthPickUp()
{
	bReplicates = true;
	
	
}
void AHealthPickUp::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
	if(ShooterCharacter)
	{
		UBuffComponent* Buff = ShooterCharacter->GetBuff();
		if(Buff)
		{
			Buff->Heal(HealAmount,HealingTime);
		}
	}
	Destroy();
}
