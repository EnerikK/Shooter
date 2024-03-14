// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Animation/AnimationAsset.h"
#include "Shooter/Types/Team.h"
#include "Weapon//WeaponTypes.h"
#include "Weapon.generated.h"


class UCombatComponent;
class AShooterPlayerController;
class AShooterCharacter;
class AAmmoEject;
class UWidgetComponent;
class USphereComponent;
class UTexture2D;
class USoundCue;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EW_Initial				UMETA(DisplayName = "InitialState"),
	EW_Equipped				UMETA(DisplayName = "Equipped"),
	EW_Dropped				UMETA(DisplayName = "Dropped"),
	EW_EquippedSecondary	UMETA(DisplayName = "EquippedSecondary"),
	
	EW_MAX					UMETA(DisplayName = "DefaultMAX")
};
UENUM(BlueprintType)
enum class EFireType : uint8
{
	EF_HitScan      UMETA(DisplayName = "Hit Scan Weapon"),
	EF_Projectile   UMETA(DisplayName = "Projectile Weapon"),
	EF_Shotgun		UMETA(DisplayName = "Shotgun Weapon"),
	
	EF_MAX			UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class SHOOTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:
	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	void ShowPickUpWidget(bool bShowWidget);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	virtual void Fire(const FVector& HitTarget);
	FVector TraceEndWithScatter(const FVector& HitTarget);
	virtual void Dropped();
	void SetHudAmmo();
	void AddAmmo(int32 AmmoToAdd);
	void SetWeaponState(EWeaponState State);
	
	FORCEINLINE USphereComponent* GetPickUpSphere() const {return PickUpSphere;}
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh()const {return WeaponMesh;}
	FORCEINLINE UWidgetComponent* GetPickUpWidget() const {return PickUpWidget;}
	FORCEINLINE float GetZoomedPov() const {return ZoomPOV;}
	FORCEINLINE float GetZoomInterpPov() const {return ZoomInterpSpeed;}
	FORCEINLINE EWeaponType GetWeaponType() const {return WeaponType;}
	FORCEINLINE int32 GetAmmo() const {return Ammo;}
	FORCEINLINE int32 GetMagCapacity() const {return MagCapacity;}
	FORCEINLINE float GetDamage() const {return Damage;}
	FORCEINLINE float GetHeadShotDamage() const {return HeadShotDamage;}
	FORCEINLINE ETeam GetTeam() const {return Team;}
	bool IsEmpty();
	bool IsFull();

	/*
	 * AutoFire rifle basically hold fire 
	 */
	UPROPERTY(EditAnywhere,Category="Combat")
	float FireDelay = .15f;

	UPROPERTY(EditAnywhere,Category="Combat")
	bool bIsAutomatic = true;

	bool bDestroyWeapon = false;

	UPROPERTY(EditAnywhere,Category="Weapon Scatter")
	bool bUseScatter = false;

	/*
	* Texture for the cross-hairs
	*/
	UPROPERTY(EditAnywhere,Category="Crosshair")
	UTexture2D* CrosshairCenter;

	UPROPERTY(EditAnywhere,Category="Crosshair")
	UTexture2D* CrosshairLeft;

	UPROPERTY(EditAnywhere,Category="Crosshair")
	UTexture2D* CrosshairRight;

	UPROPERTY(EditAnywhere,Category="Crosshair")
	UTexture2D* CrosshairTop;

	UPROPERTY(EditAnywhere,Category="Crosshair")
	UTexture2D* CrosshairBottom;
	/*
	* Texture for the cross-hairs end
	*/
	UPROPERTY(EditAnywhere)
	USoundCue* EquipSound;

	/*
	 * CustomDepth
	 */
	void EnableCustomDepth(bool bEnable);

	UPROPERTY(EditAnywhere)
	EFireType FireType;
	
protected:
	
	virtual void BeginPlay() override;
	virtual void OnWeaponStateSet();
	virtual void OnEquipped();
	virtual void OnDropped();
	virtual void OnEquippedSecondary();
	
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,
		bool bFromSweep,const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex);

	/*
	* Trace end with scatter 
	*/
	UPROPERTY(EditAnywhere,Category="Weapon Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere,Category="Weapon Scatter")
	float SphereRadius = 75.f;
	
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f;

	UPROPERTY(Replicated,EditAnywhere)
	bool bUseServerSideRewind = false;

	UPROPERTY()
	AShooterCharacter* ShooterOwnerCharacter;
	
	UPROPERTY()
	AShooterPlayerController* ShooterOwnerPlayerController;

	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);

private:

	

	UPROPERTY(VisibleAnywhere,Category="Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere,Category="Weapon Properties")
	USphereComponent* PickUpSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState,VisibleAnywhere,Category="Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere,Category="Weapon Properties")
	UWidgetComponent* PickUpWidget;

	UPROPERTY(EditAnywhere , Category ="Weapon Properties")
	UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AAmmoEject> AmmoClass;

	UPROPERTY(EditAnywhere)
	int32 Ammo;

	UFUNCTION(Client,Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client,Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);

	UPROPERTY(EditAnywhere)
	int32 MagCapacity;

	/*The number of unprocessed server requests for ammo incremented in spend round and decremented in client update ammo*/
	int32 Sequence = 0;

	void SpendRound(); //shoot bullet
	
	/*
	 * Zoom Pov WhileAiming
	 */
	UPROPERTY(EditAnywhere)
	float ZoomPOV = 30.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere)
	ETeam Team;

	
	
};
