// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


class AShooterCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOOTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	friend AShooterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	void Heal(float HealAmount,float HealTime);
	void ReplenishShield(float ShieldAmount,float ReplenishTime);
	void BuffSpeed(float BuffBaseSpeed,float BuffCrouchSpeed,float BuffTime);
	void BuffJump(float BuffJumpVelocity,float BuffDuration);
	void SetInitialSpeed(float BaseSpeed,float BaseCrouchSpeed);
	void SetInitialJumpVelocity(float Velocity);

protected:
	
	virtual void BeginPlay() override;
	void HealRampUp(float DeltaTime);
	void ShieldRampUp(float DeltaTime);

private:

	UPROPERTY()
	AShooterCharacter* Character;

	/*Heal Buff*/
	bool bHealing = false;
	float HealingRate = 0;
	float AmountToHeal = 0.f;

	/*Shield Buff*/
	bool bReplenishShield = false;
	float ShieldReplenishRate = 0.f;
	float ShieldAmountToReplenish = 0.f;

	/*Speed Buff*/
	FTimerHandle SpeedBuffTimer;
	void ResetSpeed();
	float InitialSpeed;
	float InitialCrouchSpeed;

	UFUNCTION(NetMulticast,Reliable)
	void MulticastSpeedBuff(float BaseSpeed,float CrouchSpeed);

	/*Jump Buff*/
	FTimerHandle JumpBuffTimer;
	void ResetJump();
	float InitialJumpVelocity;

	UFUNCTION(NetMulticast,Reliable)
	void MulticastJumpBuff(float JumpVelocity);

	
		
};
