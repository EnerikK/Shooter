// Hello :) 


#include "Shooter/Public/Player/ShooterPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/ShooterCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameMode.h"
#include "Hud/HudOverlay.h"
#include "Hud/ShooterHUD.h"
#include "Net/UnrealNetwork.h"

AShooterPlayerController::AShooterPlayerController()
{
	bReplicates = true;
}
void AShooterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShooterPlayerController,MatchState);
	
}
void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ShooterHUD = Cast<AShooterHUD>(GetHUD());

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if(Subsystem)
	{
		Subsystem->AddMappingContext(PlayerContext,0);
	}
}
void AShooterPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SetHudTime();
	CheckTimeSync(DeltaSeconds);
	PollInit();
	
}
void AShooterPlayerController::CheckTimeSync(float DeltaSeconds)
{
	TimeSyncRunningTime += DeltaSeconds;
	if(IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
 	{
 		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
 		TimeSyncRunningTime = 0.f;
 	}
}
void AShooterPlayerController::PollInit()
{
	if(HudOverlay == nullptr)
	{
		if(ShooterHUD && ShooterHUD->HudOverlay)
		{
			HudOverlay = ShooterHUD->HudOverlay;
			if(HudOverlay)
			{
				SetHudHealth(HudHealth,HudMaxHealth);
				SetHudScore(HudScore);
				SetHudDefeats(HudDefeats);
			}
		}
	}
	
}
void AShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(InPawn);
	if(ShooterCharacter)
	{
		SetHudHealth(ShooterCharacter->GetHealth(),ShooterCharacter->GetMaxHealth());
	}
	
}
void AShooterPlayerController::SetHudHealth(float Health, float MaxHealth)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD && ShooterHUD->HudOverlay && ShooterHUD->HudOverlay->HealthBar && ShooterHUD->HudOverlay->HealthText;
	if(bHudValid)
	{
		const float HealthPercent = Health/MaxHealth;
		ShooterHUD->HudOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"),FMath::CeilToInt(Health),FMath::CeilToInt(MaxHealth));
		ShooterHUD->HudOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeHudOverlay = true;
		HudHealth = Health;
		HudMaxHealth = MaxHealth;
	}
	
}
void AShooterPlayerController::SetHudScore(float Score)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD && ShooterHUD->HudOverlay && ShooterHUD->HudOverlay->ScoreAmount;
	if(bHudValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"),FMath::FloorToInt(Score));
		ShooterHUD->HudOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeHudOverlay = true;
		HudScore = Score;
	}
}
void AShooterPlayerController::SetHudDefeats(int32 Defeats)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD && ShooterHUD->HudOverlay && ShooterHUD->HudOverlay->DefeatsAmount;
	if(bHudValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"),Defeats);
		ShooterHUD->HudOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
		
	}
	else
	{
		bInitializeHudOverlay = true;
		HudDefeats = Defeats;
	}
}
void AShooterPlayerController::SetHudWeaponAmmo(int32 Ammo)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD && ShooterHUD->HudOverlay && ShooterHUD->HudOverlay->WeaponAmmoAmount;
	if(bHudValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"),Ammo);
		ShooterHUD->HudOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
		
	}
	
}
void AShooterPlayerController::SetHudCarriedAmmo(int32 Ammo)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD && ShooterHUD->HudOverlay && ShooterHUD->HudOverlay->CarriedAmmo;
	if(bHudValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"),Ammo);
		ShooterHUD->HudOverlay->CarriedAmmo->SetText(FText::FromString(AmmoText));
		
	}
}
void AShooterPlayerController::SetHudMatchCountdown(float CountDownTime)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD && ShooterHUD->HudOverlay && ShooterHUD->HudOverlay->MatchCountdownText;
	if(bHudValid)
	{
		int32 Minutes = FMath::FloorToInt(CountDownTime / 60.f);
		int32 Seconds = CountDownTime - Minutes * 60;
		
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"),Minutes,Seconds);
		ShooterHUD->HudOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
		
	}
}

float AShooterPlayerController::GetServerTime()
{
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AShooterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if(IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
	
}
void AShooterPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;
	if(MatchState == MatchState::InProgress)
	{
		ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
		if(ShooterHUD)
		{
			ShooterHUD->AddHudOverlay();
		}
	}
}
void AShooterPlayerController::OnRep_MatchState()
{
	if(MatchState == MatchState::InProgress)
	{
		ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
		if(ShooterHUD)
		{
			ShooterHUD->AddHudOverlay();
		}
	}
}
void AShooterPlayerController::SetHudTime()
{
	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetServerTime());
	if(CountdownInt != SecondsLeft)
	{
		SetHudMatchCountdown(MatchTime - GetServerTime());
	}
	CountdownInt = SecondsLeft;
	
}
void AShooterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceive = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest,ServerTimeOfReceive);
}
void AShooterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
	float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
	
}
void AShooterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	
	EnhancedInputComponent->BindAction(
		MoveAction,ETriggerEvent::Triggered,this,&AShooterPlayerController::Move);
		
	EnhancedInputComponent->BindAction(
		LookAction,ETriggerEvent::Triggered,this,&AShooterPlayerController::Look);
		
	EnhancedInputComponent->BindAction(
		JumpAction,ETriggerEvent::Triggered,this,&AShooterPlayerController::Jump);

	EnhancedInputComponent->BindAction(
		EquipAction,ETriggerEvent::Triggered,this,&AShooterPlayerController::Equip);

	EnhancedInputComponent->BindAction(
	CrouchAction,ETriggerEvent::Triggered,this,&AShooterPlayerController::Crouch);
	
	EnhancedInputComponent->BindAction(
	AimAction,ETriggerEvent::Triggered,this,&AShooterPlayerController::Aim);

	EnhancedInputComponent->BindAction(
	ReleaseAimAction,ETriggerEvent::Triggered,this,&AShooterPlayerController::ReleaseAim);

	EnhancedInputComponent->BindAction(
	FireAction,ETriggerEvent::Triggered,this,&AShooterPlayerController::Fire);

	EnhancedInputComponent->BindAction(
	ReloadAction,ETriggerEvent::Triggered,this,&AShooterPlayerController::Reload);
	
}
void AShooterPlayerController::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();
	APawn* ControlledPawn = GetPawn<APawn>();
	
	if (ControlledPawn != nullptr)
	{
		const FVector2D MoveValue = Value.Get<FVector2D>();
		const FRotator MovementRotation(0, ControlledPawn->GetControlRotation().Yaw, 0);
 
		// Forward/Backward direction
		if (MoveValue.Y != 0.f)
		{
			// Get forward vector
			const FVector Direction = MovementRotation.RotateVector(FVector::ForwardVector);
			
			ControlledPawn->AddMovementInput(Direction, MoveValue.Y);
		}
 
		// Right/Left direction
		if (MoveValue.X != 0.f)
		{
			// Get right vector
			const FVector Direction = MovementRotation.RotateVector(FVector::RightVector);
 
			ControlledPawn->AddMovementInput(Direction, MoveValue.X);
		}
	}
}

void AShooterPlayerController::Look(const FInputActionValue& Value)
{
	APawn* ControlledPawn = GetPawn<APawn>();
	
	if (ControlledPawn->Controller != nullptr)
	{
		const FVector2D LookValue = Value.Get<FVector2D>();
 
		if (LookValue.X != 0.f)
		{
			ControlledPawn->AddControllerYawInput(LookValue.X);
			
		}
 
		if (LookValue.Y != 0.f)
		{
			ControlledPawn->AddControllerPitchInput(LookValue.Y);
		}
	}
}
void AShooterPlayerController::Jump(const FInputActionValue& Value)
{
	if(ACharacter* ControlledCharacter = GetPawn<ACharacter>())
	{
		ControlledCharacter->Jump();
	}
}
void AShooterPlayerController::Equip(const FInputActionValue& Value)
{
	AShooterCharacter* ControlledCharacter = Cast<AShooterCharacter>(GetCharacter());
	ControlledCharacter->EquipButtonPressed();
}
void AShooterPlayerController::Crouch(const FInputActionValue& Value)
{
	AShooterCharacter* ControlledCharacter = Cast<AShooterCharacter>(GetCharacter());
	ControlledCharacter->CrouchButtonPressed();
	
}
void AShooterPlayerController::Aim(const FInputActionValue& Value)
{
	AShooterCharacter* ControlledCharacter = Cast<AShooterCharacter>(GetCharacter());
	ControlledCharacter->AimButtonPressed();
}
void AShooterPlayerController::ReleaseAim(const FInputActionValue& Value)
{
	AShooterCharacter* ControlledCharacter = Cast<AShooterCharacter>(GetCharacter());
	ControlledCharacter->AimButtonReleased();
}
void AShooterPlayerController::Fire(const FInputActionValue& Value)
{
	AShooterCharacter* ControlledCharacter = Cast<AShooterCharacter>(GetCharacter());
	ControlledCharacter->FireButtonPressed();
}

void AShooterPlayerController::Reload(const FInputActionValue& Value)
{
	AShooterCharacter* ControlledCharacter = Cast<AShooterCharacter>(GetCharacter());
	ControlledCharacter->ReloadButtonPressed();
}
	
