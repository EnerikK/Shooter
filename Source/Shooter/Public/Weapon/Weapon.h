// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Animation/AnimationAsset.h"
#include "Weapon//WeaponTypes.h"
#include "Weapon.generated.h"


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
	EW_Initial	UMETA(DisplayName = "InitialState"),
	EW_Equipped	UMETA(DisplayName = "Equipped"),
	EW_Dropped	UMETA(DisplayName = "Dropped"),

	EW_MAX		UMETA(DisplayName = "DefaultMAX"),
	
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
	void Dropped();
	void SetHudAmmo();
	void AddAmmo(int32 AmmoToAdd);
	
	FORCEINLINE void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetPickUpSphere() const {return PickUpSphere;}
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh()const {return WeaponMesh;}
	FORCEINLINE float GetZoomedPov() const {return ZoomPOV;}
	FORCEINLINE float GetZoomInterpPov() const {return ZoomInterpSpeed;}
	FORCEINLINE EWeaponType GetWeaponType() const {return WeaponType;}
	FORCEINLINE int32 GetAmmo() const {return Ammo;}
	FORCEINLINE int32 GetMagCapacity() const {return MagCapacity;}
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
	
protected:
	
	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,
		bool bFromSweep,const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex);

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

	UPROPERTY(EditAnywhere,ReplicatedUsing=OnRep_Ammo)
	int32 Ammo;

	UPROPERTY(EditAnywhere)
	int32 MagCapacity;

	UFUNCTION()
	void OnRep_Ammo();

	void SpendRound(); //shoot bullet

	UPROPERTY()
	AShooterCharacter* ShooterOwnerCharacter;
	
	UPROPERTY()
	AShooterPlayerController* ShooterOwnerPlayerController;

	/*
	 * Zoom Pov WhileAiming
	 */
	UPROPERTY(EditAnywhere)
	float ZoomPOV = 30.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;
		
};
