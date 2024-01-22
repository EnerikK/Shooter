// Hello :) 


#include "Shooter/Public/Player/ShooterPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/ShooterCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/Character.h"
#include "Hud/HudOverlay.h"
#include "Hud/ShooterHUD.h"


AShooterPlayerController::AShooterPlayerController()
{
	bReplicates = true;
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
	
