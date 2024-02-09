// Hello :) 


#include "PickUp/JumpPickUp.h"
#include "Character/ShooterCharacter.h"
#include "Components/BuffComponent.h"

void AJumpPickUp::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
	if(ShooterCharacter)
	{
		UBuffComponent* Buff = ShooterCharacter->GetBuff();
		if(Buff)
		{
			Buff->BuffJump(JumpZVelocityBuff,JumpBuffDuration);
		}
	}
	Destroy();
}
