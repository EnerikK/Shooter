// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Animation/AnimationAsset.h"
#include "Weapon.generated.h"

class AAmmoEject;
class UWidgetComponent;
class USphereComponent;
class UTexture2D;

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
	virtual void Fire(const FVector& HitTarget);
	void Dropped();
	
	FORCEINLINE void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetPickUpSphere() const {return PickUpSphere;}
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh()const {return WeaponMesh;}
	FORCEINLINE float GetZoomedPov() const {return ZoomPOV;}
	FORCEINLINE float GetZoomInterpPov() const {return ZoomInterpSpeed;}

	/*
	 * AutoFire rifle basically hold fire 
	 */
	UPROPERTY(EditAnywhere,Category="Combat")
	float FireDelay = .15f;

	UPROPERTY(EditAnywhere,Category="Combat")
	bool bIsAutomatic = true;

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

	UPROPERTY(EditAnywhere , Category = "Weapon Properties")
	UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AAmmoEject> AmmoClass;

	/*
	 * Zoom Pov WhileAiming
	 */
	UPROPERTY(EditAnywhere)
	float ZoomPOV = 30.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;
		
};
