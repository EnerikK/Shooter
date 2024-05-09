// Hello :) 


#include "Components/BuffComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/ShooterCharacter.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}
void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

	
}
void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	HealRampUp(DeltaTime);
	ShieldRampUp(DeltaTime);

}
void UBuffComponent::Heal(float HealAmount, float HealTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealTime;
	AmountToHeal += HealAmount;
}
void UBuffComponent::ReplenishShield(float ShieldAmount, float ReplenishTime)
{
	bReplenishShield = true;
	ShieldReplenishRate = ShieldAmount / ReplenishTime;
	ShieldAmountToReplenish += ShieldAmount;
}
void UBuffComponent::HealRampUp(float DeltaTime)
{
	if(!bHealing || Character == nullptr || Character->IsElimmed()) return;
	
	const float HealThisFrame = HealingRate * DeltaTime;
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame,0.f,Character->GetMaxHealth()));
	Character->UpdateHudHealth();
	AmountToHeal -= HealThisFrame;

	if(AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}
void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if(!bReplenishShield || Character == nullptr || Character->IsElimmed()) return;
	
	const float ReplenishThisFrame = ShieldReplenishRate * DeltaTime;
	Character->SetShield(FMath::Clamp(Character->GetShield() + ReplenishThisFrame,0.f,Character->GetMaxShield()));
	Character->UpdateHudShield();
	ShieldAmountToReplenish -= ReplenishThisFrame;

	if(ShieldAmountToReplenish <= 0.f || Character->GetShield() >= Character->GetMaxShield())
	{
		bReplenishShield = false;
		ShieldAmountToReplenish = 0.f;
	}
}
void UBuffComponent::SetInitialSpeed(float BaseSpeed, float BaseCrouchSpeed)
{
	InitialSpeed = BaseSpeed;
	InitialCrouchSpeed = BaseCrouchSpeed;
}
void UBuffComponent::SetInitialJumpVelocity(float Velocity)
{
	InitialJumpVelocity = Velocity;
}
void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	if(Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(SpeedBuffTimer,this,&UBuffComponent::ResetSpeed,BuffTime);
	if(Character->GetShooterCharacterComponent())
	{
		Character->GetShooterCharacterComponent()->Walk_MaxWalkSpeed = BuffBaseSpeed;
		Character->GetShooterCharacterComponent()->MaxWalkSpeedCrouched = BuffCrouchSpeed;
	}
	MulticastSpeedBuff(BuffBaseSpeed,BuffCrouchSpeed);
}
void UBuffComponent::ResetSpeed()
{
	if(Character == nullptr || Character->GetShooterCharacterComponent() == nullptr) return;

	Character->GetShooterCharacterComponent()->Walk_MaxWalkSpeed = InitialSpeed;
	Character->GetShooterCharacterComponent()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
	MulticastSpeedBuff(InitialSpeed,InitialCrouchSpeed);
}
void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	if(Character && Character->GetShooterCharacterComponent())
	{	
		Character->GetShooterCharacterComponent()->Walk_MaxWalkSpeed = BaseSpeed;
		Character->GetShooterCharacterComponent()->MaxWalkSpeedCrouched = CrouchSpeed;
	}
}
void UBuffComponent::ResetJump()
{
	if(Character->GetShooterCharacterComponent())
	{
		Character->GetShooterCharacterComponent()->JumpZVelocity = InitialJumpVelocity;
	}
	MulticastJumpBuff(InitialJumpVelocity);
}
void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffDuration)
{
	if(Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(JumpBuffTimer,this,&UBuffComponent::ResetJump,BuffDuration);
	if(Character->GetShooterCharacterComponent())
	{
		Character->GetShooterCharacterComponent()->JumpZVelocity = BuffJumpVelocity;
	}
	MulticastJumpBuff(BuffJumpVelocity);
}
void UBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity)
{
	if(Character && Character->GetShooterCharacterComponent())
	{
		Character->GetShooterCharacterComponent()->JumpZVelocity = JumpVelocity;
	}
}


