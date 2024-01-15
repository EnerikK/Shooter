// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Hud/ShooterHUD.h"
#include "CombatComponent.generated.h"

#define TRACE 80000.f

class AShooterHUD;
class AWeapon;
class AShooterPlayerController;
class AShooterCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOOTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	UCombatComponent();
	friend AShooterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	void EquipWeapon(AWeapon* WeaponToEquip);
	
protected:

	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server,Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();
	void Fire();

	void FireButtonPressed(bool bPressed);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast,Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshair(FHitResult& TraceHitResult);

	void SetHudCrosshair(float DeltaTime);
	
private:
	
	AShooterCharacter* Character;

	AShooterPlayerController* Controller;

	AShooterHUD* Hud;
	
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	FVector HitTarget;

	FHUDPackage HUDPackage;

	/*
	 * Crosshair
	 */

	float CrosshairVelocity;
	float CrosshairInAir;
	float CrosshairAim;
	float CrosshairShooting;

	/*
	 * Aiming Pov 
	 */
	float DefaultPov;

	UPROPERTY(EditAnywhere,Category="Combat")
	float ZoomedPov = 30.f;

	float CurrentPov; 
	
	UPROPERTY(EditAnywhere,Category="Combat")
	float ZoomInterpSpeed = 20.f;

	void InterpPov(float DeltaTime);

	/*
	 * AutoFire rifle basically hold fire 
	 */
	FTimerHandle Firetimer;
	
	bool bCanFire = true;

	void StartFiretimer();
	void FireTimerFinished();
	
};
