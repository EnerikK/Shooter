// Hello :) 


#include "Weapon/ProjectileMovement.h"

UProjectileMovementComponent::EHandleBlockingHitResult UProjectileMovement::HandleBlockingHit(const FHitResult& Hit,
	float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);
	return  EHandleBlockingHitResult::AdvanceNextSubstep;
}

void UProjectileMovement::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	//Rockets should not stop only explode when collision Detects a hit.
}
