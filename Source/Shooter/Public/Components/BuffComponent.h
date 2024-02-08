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
	void BuffSpeed(float BuffBaseSpeed,float BuffCrouchSpeed,float BuffTime);
	void SetInitialSpeed(float BaseSpeed,float BaseCrouchSpeed);
	

protected:
	virtual void BeginPlay() override;
	void HealRampUp(float DeltaTime);

private:

	UPROPERTY()
	AShooterCharacter* Character;

	/*Heal Buff*/
	bool bHealing = false;
	float HealingRate = 0;
	float AmountToHeal = 0.f;

	/*Speed Buff*/
	FTimerHandle SpeedBuffTimer;
	void ResetSpeed();
	float InitialSpeed;
	float InitialCrouchSpeed;

	UFUNCTION(NetMulticast,Reliable)
	void MulticastSpeedBuff(float BaseSpeed,float CrouchSpeed);
		
};
