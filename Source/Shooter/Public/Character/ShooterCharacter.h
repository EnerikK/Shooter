// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Shooter/Types/TurnInPlace.h"
#include "ShooterCharacter.generated.h"

class UCombatComponent;
class AWeapon;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AShooterCharacter();
		
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void FireButtonPressed();
	void FireButtonReleased();
	
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	FORCEINLINE float GetAO_Yaw() const {return AO_Yaw;}
	FORCEINLINE float GetAO_Pitch() const {return  AO_Pitch;}
	AWeapon* GetEquippedWeapon();

	FORCEINLINE ETurnInPlace GetTurningInPlace() const {return TurningInPlace;}

protected:
	
	virtual void BeginPlay() override;
	void AimOffset(float DeltaTime);

private:

	UPROPERTY(VisibleAnywhere,Category="Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere,Category="Camera")
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere)
	UCombatComponent* Combat;
	
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;
	
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION(Server,Reliable)
	void ServerEquipButtonPressed();

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurnInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	UPROPERTY(EditAnywhere , Category= "Combat")
	UAnimMontage* FireWeaponMontage;
	

};
