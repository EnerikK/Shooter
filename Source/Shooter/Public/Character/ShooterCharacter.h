// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "Components/LagCompensationComponent.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "Interface/InteractInterface.h"
#include "Shooter/Types/TurnInPlace.h"
#include "Shooter/Types/CombatState.h"
#include "Shooter/Types/Team.h"
#include "ShooterCharacter.generated.h"

class AShooterGameModeBase;
class UNiagaraComponent;
class UNiagaraSystem;
class ULagCompensationComponent;
class UBoxComponent;
class UBuffComponent;
class ALagCompensationComponent;
class AShooterPlayerState;
class AShooterPlayerController;
class UCombatComponent;
class AWeapon;
class UCameraComponent;
class USpringArmComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

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
	virtual void Destroyed() override;
	/*Montages*/
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlaySwapMontage();
	void PlayElimMontage();
	void PlayHitReactMontage();
	void PlayThrowGrenadeMontage() const;
	void PlaySlideMontage();

	
	void UpdateHudHealth();
	void UpdateHudShield();
	void UpdateHudAmmo();
	
	void Elim(bool bPlayerLeftGame);
	UFUNCTION(NetMulticast,Reliable)
	void MulticastElim(bool bPlayerLeftGame);
	void RotateInPlace(float DeltaTime);
	
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void FireButtonPressed();
	void FireButtonReleased();
	void ReloadButtonPressed();
	void GrenadeButtonPressed();
	void SlideButtonPressed();
	
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	bool IsSliding();
	bool bFinishedSwapping = false;
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);
	void SpawnDefaultWeapon();
	void SetTeamColor(ETeam Team);
	
	FORCEINLINE float GetAO_Yaw() const {return AO_Yaw;}
	FORCEINLINE float GetAO_Pitch() const {return  AO_Pitch;}
	FORCEINLINE ETurnInPlace GetTurningInPlace() const {return TurningInPlace;}
	AWeapon* GetEquippedWeapon();
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const {return FollowCamera;}
	FORCEINLINE bool IsElimmed() const {return bIsElimmed;}
	FORCEINLINE float GetHealth() const {return Health;}
	FORCEINLINE void SetHealth(float Amount) {Health = Amount;}
	FORCEINLINE float GetMaxHealth() const {return MaxHealth;}
	FORCEINLINE float GetShield() const {return Shield;}
	FORCEINLINE void SetShield(float Amount) {Shield = Amount;}
	FORCEINLINE float GetMaxShield() const {return MaxShield;}
	FORCEINLINE UCombatComponent* GetCombat() const {return Combat;}
	FORCEINLINE UBuffComponent* GetBuff() const {return Buff;}
	FORCEINLINE UAnimMontage* GetReloadMontage() const {return ReloadMontage;}
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const {return AttachedGrenade;}
	FORCEINLINE ULagCompensationComponent* GetLagCompensation() const {return LagCompensation;}
	FORCEINLINE bool IsHoldingFlag() const;

	void SetHoldingFlag(bool bHolding);
	
	ECombatState GetCombatState() const;
	bool IsLocallyReloading();

	ETeam GetTeam();

	/*Hit Boxes used for server-side rewind*/

	UPROPERTY(EditAnywhere)
	UBoxComponent* Head;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Pelvis;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Spine_02;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Spine_03;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Upperarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Upperarm_r;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* Lowerarm_r;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* Lowerarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Hand_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Hand_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Thigh_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Thigh_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Calf_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Calf_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Foot_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Foot_r;

	UPROPERTY(EditAnywhere)
	TMap<FName,UBoxComponent*> HitCollisionBoxes;

	UFUNCTION(Server,Reliable)
	void ServerLeaveGame();

	FOnLeftGame OnLeftGame;

	UFUNCTION(NetMulticast,Reliable)
	void MulticastGainedTheLead();
	UFUNCTION(NetMulticast,Reliable)
	void MulticastLostTheLead();

protected:
	
	virtual void BeginPlay() override;
	void AimOffset(float DeltaTime);

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor,float Damage,const UDamageType* DamageType,AController* InstigatorController,AActor* DamageCauser);
	//Poll for any relevant classes and initialiaze the class
	void PollInit();
	void DropOrDestroyWeapon(AWeapon* Weapon);
	void DropOrDestroyWeapons();
	void SetSpawnPoint();
	void OnPlayerStateInitialized();

private:

	UPROPERTY()
	AShooterPlayerState* ShooterPlayerState;

	UPROPERTY()
	AShooterPlayerController* ShooterPlayerController;

	UPROPERTY(VisibleAnywhere,Category="Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere,Category="Camera")
	UCameraComponent* FollowCamera;

	/*Components*/
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,meta = (AllowPrivateAccess = true))
	UCombatComponent* Combat;

	UPROPERTY(VisibleAnywhere)
	UBuffComponent* Buff;

	UPROPERTY(VisibleAnywhere)
	ULagCompensationComponent* LagCompensation;
	
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

	/*Elim Effect*/

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* CrownSystem;

	UPROPERTY()
	UNiagaraComponent* CrownComponent;

	/*
	 * Montages
	 */
	UPROPERTY(EditAnywhere , Category= "Combat")
	UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere , Category= "Combat")
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere , Category= "Combat")
	UAnimMontage* ElimMontage;

	UPROPERTY(EditAnywhere , Category= "Combat")
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere , Category= "Combat")
	UAnimMontage* GrenadeToss;

	UPROPERTY(EditAnywhere , Category= "Combat")
	UAnimMontage* Slide;

	UPROPERTY(EditAnywhere , Category= "Combat")
	UAnimMontage* Swap;
	
	void HideCamera();
	
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;
	
	/*
	 * PlayerStats
	 */
	/*Player Health*/
	UPROPERTY(EditAnywhere,Category="Player Stats")
	float MaxHealth = 100;

	UPROPERTY(ReplicatedUsing=OnRep_Health,VisibleAnywhere,Category="Player Stats")
	float Health = 100;

	bool bIsElimmed = false;

	UFUNCTION()
	void OnRep_Health(float LastHealth);
	
	/*Player Shield*/
	UPROPERTY(EditAnywhere,Category="Player Stats")
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing=OnRep_Shield,EditAnywhere,Category="Player Stats")
	float Shield = 0.f;

	UFUNCTION()
	void OnRep_Shield(float LastShield);
	
	//Dissolve Effect  TODO ::  there's a bug(Feature) that only dissolve on of the 2 materials that the Character Has Fix it at some point 

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
	UPROPERTY(VisibleAnywhere,Category="Elimination")
	UMaterialInstance* DissolveMaterialInstance;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	/*TeamColors*/
	UPROPERTY(EditAnywhere,Category="Elimination")
	UMaterialInstance* RedDissolveMaterialInstance;

	UPROPERTY(EditAnywhere,Category="Elimination")
	UMaterialInstance* RedMaterial;

	UPROPERTY(EditAnywhere,Category="Elimination")
	UMaterialInstance* BlueMaterial;
	
	UPROPERTY(EditAnywhere,Category="Elimination")
	UMaterialInstance* BlueDissolveMaterialInstance;

	UPROPERTY(EditAnywhere,Category="Elimination")
	UMaterialInstance* OriginalMaterial;

	//Timer
	FTimerHandle ElimTimer;

	void ElimTimerFinished();

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	bool bLeftGame = false;
	
	/*Grenade*/
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;

	/*DefaultWeapon*/

	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;

	UPROPERTY()
	AShooterGameModeBase* ShooterGameMode;
};
