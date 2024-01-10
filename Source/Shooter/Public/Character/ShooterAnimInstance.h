// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Shooter/Types/TurnInPlace.h"
#include "ShooterAnimInstance.generated.h"

class AWeapon;
class AShooterCharacter;
/**
 * 
 */
UCLASS()
class SHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:

	UPROPERTY(BlueprintReadOnly,Category="Character",meta=(AllowPrivateAccess = "true"))
	AShooterCharacter* Character;

	UPROPERTY(BlueprintReadOnly,Category="Movement",meta=(AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(BlueprintReadOnly,Category="Movement",meta=(AllowPrivateAccess = "true"))
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly,Category="Movement",meta=(AllowPrivateAccess = "true"))
	bool bWeaponEquipped;

	AWeapon* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly,Category="Movement",meta=(AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly,Category="Movement",meta=(AllowPrivateAccess = "true"))
	bool bIsCrouched;

	UPROPERTY(BlueprintReadOnly,Category="Movement",meta=(AllowPrivateAccess = "true"))
	bool bAiming;

	UPROPERTY(BlueprintReadOnly,Category="Movement",meta=(AllowPrivateAccess = "true"))
	float YawOffset;

	UPROPERTY(BlueprintReadOnly,Category="Movement",meta=(AllowPrivateAccess = "true"))
	float Lean;

	UPROPERTY(BlueprintReadOnly,Category="Movement",meta=(AllowPrivateAccess = "true"))
	float AO_Yaw;

	UPROPERTY(BlueprintReadOnly,Category="Movement",meta=(AllowPrivateAccess = "true"))
	float AO_Pitch;

	UPROPERTY(BlueprintReadOnly,Category="Movement",meta=(AllowPrivateAccess = "true"))
	FTransform LeftHandTransform;
	
	UPROPERTY(BlueprintReadOnly,Category="Movement",meta=(AllowPrivateAccess = "true"))
	ETurnInPlace TurningInPlace;
	
	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotation;
	FRotator DeltaRotation;

	
};
