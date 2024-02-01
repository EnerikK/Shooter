// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ProjectileMovement.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API UProjectileMovement : public UProjectileMovementComponent
{
	GENERATED_BODY()

protected:

	virtual EHandleBlockingHitResult HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining) override;
	virtual void HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta) override;
	
};
