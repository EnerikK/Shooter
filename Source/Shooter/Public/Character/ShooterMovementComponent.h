// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ShooterMovementComponent.generated.h"

class AShooterCharacter;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSlideStartDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDashStartDelegate);

UENUM()
enum EMovementModeBattleMage
{
	BMove_None      UMETA(DisplayName = "None"),
	BMove_Slide     UMETA(DisplayName = "Slide"),
	BMove_Jump		UMETA(DisplayName = "Jumping"),
	BMove_Walking   UMETA(DisplayName = "Walking"),
	BMove_Falling   UMETA(DisplayName = "Falling"),
	
	BMove_MAX     UMETA(DisplayName = "MAX"),

};

/**
 * 
 */
UCLASS()
class SHOOTER_API UShooterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	class FSavedMove_ShooterCharacter : public FSavedMove_Character
	{
	public:
		enum CompressedFlags
		{
			Flag_Sprint	 = 0x10,
			Flag_Dash	 = 0x20,
			Flag_Custom1 = 0x30,
			Flag_Custom2 = 0x40,

		};

		typedef FSavedMove_Character Super;

		uint8 Saved_bWantsToSprint : 1;
		uint8 Saved_bWantsToSlide : 1;
		uint8 Saved_bPressedShooterJump :1;
		uint8 Saved_bWantsToDash;

		uint8 Saved_bHadAnimRootMotion : 1;
		uint8 Saved_bTransitionFinished : 1;


		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* C) override;
	};
	class FNetworkPredictionData_Client_ShooterCharacter : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_ShooterCharacter(const UCharacterMovementComponent& ClientMovement);
		typedef FNetworkPredictionData_Client_Character Super;
		virtual FSavedMovePtr AllocateNewMove() override;
		
	};

	
	/*Slide*/
	UPROPERTY(EditDefaultsOnly)
	float Slide_EnterImpulse = 5000;
	UPROPERTY(EditDefaultsOnly)
	float Slide_CooldownDuration = 1.3;
	UPROPERTY(EditDefaultsOnly)
	float AuthSlideCooldownDuration= 3.9f;
	UPROPERTY(EditDefaultsOnly)
	float SlideHalfHeight = 55.f;

	/*Mantle*/
	UPROPERTY(EditDefaultsOnly)
	float MantleMaxDistance = 200;
	UPROPERTY(EditDefaultsOnly)
	float MantleReachHeight = 50;
	UPROPERTY(EditDefaultsOnly)
	float MinMantle = 30;
	UPROPERTY(EditDefaultsOnly)
	float MantleMinWallSteepnessAngle = 75;
	UPROPERTY(EditDefaultsOnly)
	float MantleMaxSurfaceAngle = 40;
	UPROPERTY(EditDefaultsOnly)
	float MantleMaxAlignmentAngle = 45;
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* TallMantleMontage;
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* TransitionTallMantleMontage;
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* ProxyTallMantleMontage;
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* ShortMantleMontage;
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* TransitionShortMantleMontage;
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* ProxyShortMantleMontage;

	/*Dash*/
	UPROPERTY(EditDefaultsOnly)
	float DashImpulse = 1000.f;
	UPROPERTY(EditDefaultsOnly)
	float DashCooldown = 1.f;
	UPROPERTY(EditDefaultsOnly)
	float AuthDashCooldownDuration = .9f;


	bool bWantsToSprint;
	bool bPrev_WantsToCrouch;
	bool bWantsToSlide;
	bool bWantsToDash;
	bool bHadAnimRootMotion;
	bool bTransitionFinished;
	
	TSharedPtr<FRootMotionSource_MoveToForce> TransitionRMS;
	UPROPERTY(Transient)
	UAnimMontage* TransitionQueuedMontage;
	FString TransitionName;
	float TransitionQueuedMontageSpeed;
	int TransitionRMS_ID;
	
	float SlideStartTime;
	float DashStartTime;

	/*Timers*/
	FTimerHandle TimerHandle_SlideCooldown;
	FTimerHandle TimerHandle_DashCooldown;

	/*Delegates*/
	UPROPERTY(BlueprintAssignable)
	FSlideStartDelegate SlideStartDelegate;
	
	UPROPERTY(BlueprintAssignable)
	FDashStartDelegate DashStartDelegate;

	/*Replicated Variables*/
	UPROPERTY(ReplicatedUsing = OnRep_SlideStart)
	bool Proxy_bSlideStart;

	UPROPERTY(ReplicatedUsing = OnRep_DashStart)
	bool Proxy_bDashStart;

	UPROPERTY(ReplicatedUsing= OnRep_ShortMantle)
	bool Proxy_bShortMantle;
	UPROPERTY(ReplicatedUsing = OnRep_TallMantle)
	bool Proxy_bTallMantle;

public:

	/*Sprint*/
	UPROPERTY(EditDefaultsOnly)
	float Sprint_MaxWalkSpeed;
	UPROPERTY(EditDefaultsOnly)
	float Walk_MaxWalkSpeed;

	UShooterMovementComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual bool IsMovingOnGround() const override;
	virtual bool CanCrouchInCurrentState() const override;

	FORCEINLINE bool IsSliding() const {return IsCustomMovementMode(BMove_Slide);}
	

	UFUNCTION()
	void SprintPressed();

	UFUNCTION()
	void SprintReleased();

	UFUNCTION()
	void CrouchPressed();

	UFUNCTION()
	void SlidePressed();

	UFUNCTION()
	void SlideReleased();

	UFUNCTION()
	void DashPressed();
	
	UFUNCTION()
	void DashReleased();

	UFUNCTION(BlueprintPure)
	bool IsCustomMovementMode(EMovementModeBattleMage InCustomMovementMode) const;

	UFUNCTION(BlueprintPure)
	bool IsMovementMode(EMovementMode InMovementMode) const;
	


protected:
	UPROPERTY(Transient)
	AShooterCharacter* PlayerCharacter;

	virtual void InitializeComponent() override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;

	void SetCollisionSizeToSliding(float Size);
	bool RestoreDefaultCollision();

private:

	/*Slide*/
	UFUNCTION()
	void OnRep_SlideStart();
	void OnSlideCooldownFinished();
	bool CanSlide() const;
	void PerformSlide();

	/*Dash*/
	UFUNCTION()
	void OnRep_DashStart();
	bool CanDash() const;
	void PerformDash();
	void OnDashCooldownFinished();

	/*Mantle*/
	bool TryMantle();
	FVector GetMantleStartLocation(FHitResult FrontHit, FHitResult SurfaceHit, bool bTallMantle) const;
	UFUNCTION()
	void OnRep_ShortMantle();
	UFUNCTION()
	void OnRep_TallMantle();

	bool IsServer() const;
	float CapR() const;
	float CapHH() const;
		
};

