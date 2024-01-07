// Hello :) 


#include "Shooter/Public/Player/ShooterPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/ShooterCharacter.h"
#include "GameFramework/Character.h"




AShooterPlayerController::AShooterPlayerController()
{
	bReplicates = true;
}
void AShooterPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}
void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if(Subsystem)
	{
		Subsystem->AddMappingContext(PlayerContext,0);
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
	
