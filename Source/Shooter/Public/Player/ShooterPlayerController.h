// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"


class AWeapon;
class AShooterCharacter;
class UCombatComponent;
struct FInputActionValue;
class UInputMappingContext;
class UInputAction;
/**
 * 
 */
UCLASS()
class SHOOTER_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	AShooterPlayerController();
	virtual void Tick(float DeltaSeconds) override;


protected:

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:

	UPROPERTY(VisibleAnywhere)
	UCombatComponent* Combat;
	
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputMappingContext> PlayerContext;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> EquipAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> CrouchAction;
	
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> ReleaseAimAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> FireAction;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Jump(const FInputActionValue& Value);
	void Equip(const FInputActionValue& Value);
	void Crouch(const FInputActionValue& Value);
	void Aim(const FInputActionValue& Value);
	void ReleaseAim(const FInputActionValue& Value);
	void Fire(const FInputActionValue& Value);

};


