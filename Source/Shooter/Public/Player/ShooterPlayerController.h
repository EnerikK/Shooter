// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"


class AShooterGameModeBase;
class UHudOverlay;
class AShooterHUD;
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
	virtual void OnPossess(APawn* InPawn) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetHudHealth(float Health , float MaxHealth);
	void SetHudShield(float Shield , float MaxShield);
	void SetHudScore(float Score);
	void SetHudDefeats(int32 Defeats);
	void SetHudWeaponAmmo(int32 Ammo);
	void SetHudCarriedAmmo(int32 Ammo);
	void SetHudMatchCountdown(float CountdownTime);
	void SetHudAnnouncementCountdown(float CountdownTime);
	void SetHudGrenades(int32 Grenades);

	virtual float GetServerTime(); //Sync with server world clock
	virtual void ReceivedPlayer() override; //Sync With server clock as soon as possilbe
	void OnMatchStateSet(FName State);
	void HandleMatchHasStarted();
	void HandleCooldown();

	UPROPERTY()
	float SingleTripTime = 0.f;
	
	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	FORCEINLINE bool GetDisableGameplay() const {return bDisableGameplay;}
	
protected:

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	void SetHudTime();

	/*
	 * Syncing Time between client and server 
	 */
	//Reqeusts The Current ServerTime , passing the clients time when the request was sent
	UFUNCTION(Server,Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	//Report the current server time to the client in responce to serverReqeustServerTime 
	UFUNCTION(Client,Reliable)
	void ClientReportServerTime(float TimeOfClientRequest,float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f; //Diffrence Between client and server time ;

	UPROPERTY(EditAnywhere, Category="Time")
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;

	void CheckTimeSync(float DeltaSeconds);

	void PollInit();

	UFUNCTION(Server,Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client,Reliable)
	void ClientJoinMidGame(FName StateOfMatch,float Cooldown,float Intervention,float Match,float StartingTime);

	/*Lag*/
	void HighPingWarning();

	void StopHighPingWarning();

	void CheckPing(float DeltaSeconds);
private:

	UPROPERTY()
	AShooterHUD* ShooterHUD;
	
	UPROPERTY()
	AShooterGameModeBase* ShooterGameMode;

	UPROPERTY(VisibleAnywhere)
	UCombatComponent* Combat;

	float LevelStartingTime = 0.f;
	float MatchTime = 0.f;
	float InterventionTime = 0.f;
	float CooldownTime = 0.f;
	uint32 CountdownInt = 0;

	UPROPERTY(ReplicatedUsing=OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	UHudOverlay* HudOverlay;

	bool bInitializeHudOverlay = false;

	float HudHealth;
	float HudMaxHealth;
	bool bInitializeHealth = false;
	
	float HudScore;
	bool bInitializeScore = false;
	
	int32 HudDefeats;
	bool bInitializeDefeats = false;
	
	int32 HudGrenades;
	bool bInitializeGrenades = false;

	float HudShield;
	float HudMaxShield;
	bool bInitializeShield = false;

	float HudCarriedAmmo;
	float HudWeaponAmmo;
	bool bInitializeCarriedAmmo = false;
	bool bInitializeWeaponAmmo =false;

	/*Ping*/
	float HighPingRunningTime = 0.f;
	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;
	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f;
	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f;
	float PingAnimationRunningTime = 0.f;
	
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

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> ReloadAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> GrenadeToss;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> SlideAction;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Jump(const FInputActionValue& Value);
	void Equip(const FInputActionValue& Value);
	void Crouch(const FInputActionValue& Value);
	void Aim(const FInputActionValue& Value);
	void ReleaseAim(const FInputActionValue& Value);
	void Fire(const FInputActionValue& Value);
	void Reload(const FInputActionValue& Value);
	void Toss(const FInputActionValue& Value);
	void Slide(const FInputActionValue& Value);


};


