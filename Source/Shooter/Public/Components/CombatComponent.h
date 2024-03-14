// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Hud/ShooterHUD.h"
#include "Weapon/WeaponTypes.h"
#include "Shooter/Types/CombatState.h"
#include "CombatComponent.generated.h"


#define TRACE 80000.f

class AProjectile;
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

	FORCEINLINE int32 GetGrenades() const {return Grenades;}
	FORCEINLINE bool GetHoldingFlag() const {return bHoldingFlag;}
	bool bShouldSwapWeapon();

	UPROPERTY(ReplicatedUsing = OnRep_HoldingFlag)
	bool bHoldingFlag = false;
	
	UFUNCTION()
	void OnRep_HoldingFlag();

	void EquipWeapon(AWeapon* WeaponToEquip);
	void SwapWeapon();
	void Reload();
	
	UFUNCTION(BlueprintCallable)
	void FinishReloading();
	UFUNCTION(BlueprintCallable)
	void FinishSwap();
	UFUNCTION(BlueprintCallable)
	void FinishSwapAttachWeapon();
	
	void FireButtonPressed(bool bPressed);
	void SlideButtonPressed(bool bPressed);
	
	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	void JumpToShotgunEnd();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinish();

	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	UFUNCTION(Server,Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);

	void PickUpAmmo(EWeaponType WeaponType, int32 AmmoAmount);

	bool bLocallyReloading = false;


protected:

	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server,Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();
	UFUNCTION()
	void OnRep_SecondaryWeapon();
	void Fire();
	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireShotgunWeapon();
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTarget);
	
	void Slide();
	
	UFUNCTION(Server, Reliable,WithValidation)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget,float FireDelay);

	UFUNCTION(NetMulticast,Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable,WithValidation)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTarget,float FireDelay);

	UFUNCTION(NetMulticast,Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTarget);

	void TraceUnderCrosshair(FHitResult& TraceHitResult);

	void SetHudCrosshair(float DeltaTime);

	UFUNCTION(Server,Reliable)
	void ServerReload();
	
	void HandleReload();
	
	int32 AmountToReload();

	void ThrowGrenade();

	UFUNCTION(Server,Reliable)
	void ServerThrowGrenade();

	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> GrenadeClass;

	void DropEquippedWeapon();
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToLeftHand(AActor* ActorToAttach);
	void AttachFlagToLeftHand(AWeapon* Flag);
	void AttachActorToBackPack(AActor* ActorToAttach);
	void UpdateCarriedAmmo();
	void PlayEquippedSound(AWeapon* WeaponToEquip);
	void ReloadEmptyWeapon();
	void ShowAttachedGrenade(bool bShowGrenade);
	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);



	
private:

	UPROPERTY()
	AShooterCharacter* Character;
	UPROPERTY()
	AShooterPlayerController* Controller;
	UPROPERTY()
	AShooterHUD* Hud;
	
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing= OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming = false
	;

	bool bAimButtonPressed = false; // locally controlled only variable it represents the true value of whether or not we are holding the aim button

	UFUNCTION()
	void OnRep_Aiming();

	bool bSlide;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;
	
	bool bSlideButtonPressed;

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
	bool bCanSlide = true;

	void StartFiretimer();
	void FireTimerFinished();

	bool CanFire();
	bool CanSlide();

	//Carried ammo for the weapon equipped
	UPROPERTY(ReplicatedUsing=OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();
	
	TMap<EWeaponType,int32> CarriedAmmoMap; //TODO : Understand how tmaps and hash functions work better

	UPROPERTY(EditAnywhere)
	int32 MaxCarriedAmmo = 90;
	
	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;

	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 1;

	UPROPERTY(EditAnywhere)
	int32 StatingPistolAmmo = 7;
	
	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 20;

	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 5;

	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 1;

	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo = 4;
	
	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing=OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECState_Unoccupied;
	
	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValues();
	void UpdateShotgunAmmoValues();

	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 3;
	
	UFUNCTION()
	void OnRep_Grenades();
	
	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 4;
	
	void UpdateHudGrenades();

	void SetPushReplicatedVariable(bool bNewValue);

	
	
	UPROPERTY()
	AWeapon* TheFlag;
};
