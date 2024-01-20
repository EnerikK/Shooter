// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "Interface/InteractInterface.h"
#include "Shooter/Types/TurnInPlace.h"
#include "ShooterCharacter.generated.h"

class AShooterPlayerController;
class UCombatComponent;
class AWeapon;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter,public IInteractInterface
{
	GENERATED_BODY()

public:
	AShooterCharacter();
		
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	void PlayElimMontage();
	void PlayHitReactMontage();
	void UpdateHudHealth();

	void Elim();
	UFUNCTION(NetMulticast,Reliable)
	void MulticastElim();
	
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
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const {return FollowCamera;}
	FORCEINLINE bool IsElimmed() const {return bIsElimmed;}

protected:
	
	virtual void BeginPlay() override;
	void AimOffset(float DeltaTime);

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor,float Damage,const UDamageType* DamageType,AController* InstigatorController,AActor* DamageCauser);

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

	UPROPERTY(EditAnywhere , Category= "Combat")
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere , Category= "Combat")
	UAnimMontage* ElimMontage;
	
	void HideCamera();
	
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;
	
	/*
	 * PlayerStats
	 */
	UPROPERTY(EditAnywhere,Category="Player Stats")
	float MaxHealth = 100;

	UPROPERTY(ReplicatedUsing=OnRep_Health,VisibleAnywhere,Category="Player Stats")
	float Health = 100;

	bool bIsElimmed = false;

	UFUNCTION()
	void OnRep_Health();

	AShooterPlayerController* ShooterPlayerController;
	
	//Dissolve Effect  TODO ::  there's a bug that only dissolve on of the 2 materials that the Character Has Fix it at some point 

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	
	FOnTimelineFloat DissolveTrack;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	//Dynamic Instance that can be changed in runtime 
	UPROPERTY(VisibleAnywhere,Category="Elimination")
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	//Material instance set on the BP used with the dynamic material instance
	UPROPERTY(EditAnywhere,Category="Elimination")
	UMaterialInstance* DissolveMaterialInstance;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	//Timer
	FTimerHandle ElimTimer;

	void ElimTimerFinished();

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;
};
