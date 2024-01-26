// Hello :) 

#include "Shooter/Public/Character/ShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/CombatComponent.h"
#include "Game/ShooterGameModeBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Player/ShooterPlayerController.h"
#include "PlayerState/ShooterPlayerState.h"
#include "Shooter/Shooter.h"
#include "Weapon/Weapon.h"
#include "Weapon/WeaponTypes.h"


AShooterCharacter::AShooterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom,USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_Mesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);

	TurningInPlace = ETurnInPlace::ETurnIP_NotTurning;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimeLineComponent"));
	
}
void AShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(AShooterCharacter,OverlappingWeapon,COND_OwnerOnly);
	DOREPLIFETIME(AShooterCharacter,Health);
	
}
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	UpdateHudHealth();
	if(HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this,&AShooterCharacter::ReceiveDamage);
	}
}
void AShooterCharacter::UpdateHudHealth()
{
	ShooterPlayerController = ShooterPlayerController == nullptr ?  Cast<AShooterPlayerController>(Controller) : ShooterPlayerController;
	if(ShooterPlayerController)
	{
		ShooterPlayerController->SetHudHealth(Health,MaxHealth);
	}
}
void AShooterCharacter::Elim()
{
	if(Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->Dropped();
	}
	MulticastElim();
	GetWorldTimerManager().SetTimer(ElimTimer,this,&AShooterCharacter::ElimTimerFinished,ElimDelay);
}
void AShooterCharacter::MulticastElim_Implementation()
{
	if(ShooterPlayerController)
	{
		ShooterPlayerController->SetHudWeaponAmmo(0);
	}
	bIsElimmed = true;
	PlayElimMontage();
	//StartDissolve Effect
	if(DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance,this);
		GetMesh()->SetMaterial(0,DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"),-0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"),100.f);
	}
	StartDissolve();

	//DisableCharacterMovement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if(ShooterPlayerController)
	{
		DisableInput(ShooterPlayerController);
	}
	//Disable Collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
}
void AShooterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if(DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"),DissolveValue);
	}
}
void AShooterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this,&AShooterCharacter::UpdateDissolveMaterial);
	if(DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve,DissolveTrack);
		DissolveTimeline->Play();
	}
}
void AShooterCharacter::ElimTimerFinished()
{
	AShooterGameModeBase* ShooterGameMode = GetWorld()->GetAuthGameMode<AShooterGameModeBase>();
	if(ShooterGameMode)
	{
		ShooterGameMode->RequestRespawn(this,Controller);
	}

}

AWeapon* AShooterCharacter::GetEquippedWeapon()
{
	if(Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector AShooterCharacter::GetHitTarget() const
{
	if(Combat == nullptr) return FVector();
	return Combat->HitTarget;

}
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);
	HideCamera();
	PollInit();
	
}
void AShooterCharacter::AimOffset(float DeltaTime)
{
	//There's a Bug on the Yaw when you use it on Multiplayer Fix when you can if  you can future me
	if (Combat && Combat->EquippedWeapon == nullptr) return;
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) // standing still, not jumping
	{
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if(TurningInPlace == ETurnInPlace::ETurnIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir) // running, or jumping
	{
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurnInPlace::ETurnIP_NotTurning;
	}
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//found out that when i try to pitch downwards thepitch goes from 360 to 270 it should be 0 to -90 so we MAP pitch from [270,360) to [-90,0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		//Apply Correction
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}	
}
void AShooterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage,0.f,MaxHealth);
	UpdateHudHealth();
	PlayHitReactMontage();

	if(Health  == 0.f)
	{
		AShooterGameModeBase* ShooterGameMode = GetWorld()->GetAuthGameMode<AShooterGameModeBase>();
		if(ShooterGameMode)
		{
			ShooterPlayerController = ShooterPlayerController == nullptr ? Cast<AShooterPlayerController>(Controller) : ShooterPlayerController;
			AShooterPlayerController* AttackerController = Cast<AShooterPlayerController>(InstigatorController);
        	ShooterGameMode->PlayerElimination(this,ShooterPlayerController,AttackerController);	
		}
	}
}
void AShooterCharacter::PollInit()
{
	if(ShooterPlayerState == nullptr)
	{
		ShooterPlayerState = GetPlayerState<AShooterPlayerState>();
		if(ShooterPlayerState)
		{
			ShooterPlayerState->AddToScore(0.f);
			ShooterPlayerState->AddToDefeats(0);
		}
	}
}
void AShooterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if(Combat)
	{
		Combat->Character = this;
	}
	
}
void AShooterCharacter::PlayFireMontage(bool bAiming)
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}
void AShooterCharacter::PlayReloadMontage()
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssualtRifle:
			SectionName = FName("Rifle");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
	
}
void AShooterCharacter::PlayElimMontage()
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}
void AShooterCharacter::PlayHitReactMontage()
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FrontHit");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}
void AShooterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickUpWidget(false);
	}
	OverlappingWeapon = Weapon;
	if(IsLocallyControlled())
	{
		if(OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickUpWidget(true);
		}
	}
}
bool AShooterCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}
bool AShooterCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}
void AShooterCharacter::EquipButtonPressed()
{
	if(Combat)
	{
		if(HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
		
	}
}
void AShooterCharacter::CrouchButtonPressed()
{
	if(bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}
void AShooterCharacter::AimButtonPressed()
{
	if(Combat)
	{
		Combat->SetAiming(true);
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Holding!"));	
		
	}
}
void AShooterCharacter::AimButtonReleased()
{
	if(Combat)
	{
		Combat->SetAiming(false);
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Released!"));	
	}
}
void AShooterCharacter::FireButtonPressed()
{
	if(Combat)
	{
		Combat->FireButtonPressed(true);
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Firing!"));	

	}
	
}
void AShooterCharacter::FireButtonReleased()
{
	if(Combat)
	{
		Combat->FireButtonPressed(false);
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Stopped!"));	

	}
}
void AShooterCharacter::ReloadButtonPressed()
{
	if(Combat)
	{
		Combat->Reload();
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Reloading!"));	

	}
}
void AShooterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickUpWidget(true);
	}
	if(LastWeapon)
	{
		LastWeapon->ShowPickUpWidget(false);
	}
}
void AShooterCharacter::TurnInPlace(float DeltaTime)
{
	if(AO_Yaw > 90.f)
	{
		TurningInPlace = ETurnInPlace::ETurnIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurnInPlace::ETurnIP_Left;
	}
	if(TurningInPlace != ETurnInPlace::ETurnIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw,0.f,DeltaTime,4.f);
		AO_Yaw = InterpAO_Yaw;
		if(FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurnInPlace::ETurnIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
			
		}
	}
}
void AShooterCharacter::HideCamera()
{
	if(!IsLocallyControlled())return;
	if((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if(Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if(Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}
void AShooterCharacter::OnRep_Health()
{
	PlayHitReactMontage();
	UpdateHudHealth();
}
void AShooterCharacter::ServerEquipButtonPressed_Implementation()
{
	if(Combat && HasAuthority())
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
ECombatState AShooterCharacter::GetCombatState() const
{
	if(Combat == nullptr) return ECombatState::ECState_MAX;
	return Combat->CombatState;
}

