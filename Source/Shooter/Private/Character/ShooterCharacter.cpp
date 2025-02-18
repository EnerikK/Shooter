 // Hello :) 

#include "Shooter/Public/Character/ShooterCharacter.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/BuffComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/CombatComponent.h"
#include "Game/ShooterGameModeBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameState/ShooterGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Player/ShooterPlayerController.h"
#include "PlayerStarts/TeamPlayerStart.h"
#include "PlayerState/ShooterPlayerState.h"
#include "Shooter/Shooter.h"
#include "Weapon/Weapon.h"
#include "Weapon/WeaponTypes.h"


AShooterCharacter::AShooterCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UShooterMovementComponent>(
	  ACharacter::CharacterMovementComponentName))
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

	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	ShooterMovementComponent = Cast<UShooterMovementComponent>(GetCharacterMovement());
	ShooterMovementComponent->SetIsReplicated(true);

	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));
	
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_Mesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);

	TurningInPlace = ETurnInPlace::ETurnIP_NotTurning;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimeLineComponent"));

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AttachedGrenade"));
	AttachedGrenade->SetupAttachment(GetMesh());
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Health3D = CreateDefaultSubobject<UStaticMeshComponent>("3DHealth");
	Health3D->SetupAttachment(GetMesh());
	Health3D->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Shield3D = CreateDefaultSubobject<UStaticMeshComponent>("3DShield");
	Shield3D->SetupAttachment(GetMesh());
	Shield3D->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	
	/*Hit Boxes for server-side-rewind*/
	Head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	Head->SetupAttachment(GetMesh(),FName("head"));
	HitCollisionBoxes.Add(FName("head"),Head);

	Pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	Pelvis->SetupAttachment(GetMesh(),FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"),Pelvis);
	
	Spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	Spine_02->SetupAttachment(GetMesh(),FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"),Spine_02);
	
	Spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	Spine_03->SetupAttachment(GetMesh(),FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"),Spine_03);

	Upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	Upperarm_l->SetupAttachment(GetMesh(),FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"),Upperarm_l);
	
	Upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	Upperarm_r->SetupAttachment(GetMesh(),FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"),Upperarm_r);
	
	Lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	Lowerarm_l->SetupAttachment(GetMesh(),FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"),Lowerarm_l);

	Lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	Lowerarm_r->SetupAttachment(GetMesh(),FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"),Lowerarm_r);

	Hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	Hand_l->SetupAttachment(GetMesh(),FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"),Hand_l);
	
	Hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	Hand_r->SetupAttachment(GetMesh(),FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"),Hand_r);

	Thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	Thigh_l->SetupAttachment(GetMesh(),FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"),Thigh_l);

	Thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	Thigh_r->SetupAttachment(GetMesh(),FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"),Thigh_r);

	Calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	Calf_l->SetupAttachment(GetMesh(),FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"),Calf_l);

	Calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	Calf_r->SetupAttachment(GetMesh(),FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"),Calf_r);

	Foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	Foot_l->SetupAttachment(GetMesh(),FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"),Foot_l);

	Foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	Foot_r->SetupAttachment(GetMesh(),FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"),Foot_r);

	for(auto Box : HitCollisionBoxes)
	{
		if(Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox,ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
	
}
void AShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(AShooterCharacter,OverlappingWeapon,COND_OwnerOnly);
	DOREPLIFETIME(AShooterCharacter,Health);
	DOREPLIFETIME(AShooterCharacter,Shield);
}
void AShooterCharacter::MulticastGainedTheLead_Implementation()
{
	if(CrownSystem == nullptr) return;
	if(CrownComponent == nullptr)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrownSystem,
			GetCapsuleComponent(),
			FName(),
			GetActorLocation() + FVector(0.f,0.f,100.f),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false);
	}
	if(CrownComponent)
	{
		CrownComponent->Activate();
	}
}
void AShooterCharacter::MulticastLostTheLead_Implementation()
{
	if(CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
}
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	SpawnDefaultWeapon();
	UpdateHudAmmo();
	UpdateHudHealth();
	UpdateHudShield();
	SlideStartDelegate.Broadcast();
	DashStartDelegate.Broadcast();
	if(HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this,&AShooterCharacter::ReceiveDamage);
	}
	if(AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
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
void AShooterCharacter::UpdateHudShield()
{
	ShooterPlayerController = ShooterPlayerController == nullptr ?  Cast<AShooterPlayerController>(Controller) : ShooterPlayerController;
	if(ShooterPlayerController)
	{
		ShooterPlayerController->SetHudShield(Shield,MaxShield);
	}
}
void AShooterCharacter::UpdateHudAmmo()
{
	ShooterPlayerController = ShooterPlayerController == nullptr ?  Cast<AShooterPlayerController>(Controller) : ShooterPlayerController;
	if(ShooterPlayerController && Combat && Combat->EquippedWeapon)
	{
		ShooterPlayerController->SetHudCarriedAmmo(Combat->CarriedAmmo);
		ShooterPlayerController->SetHudWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	}
}
void AShooterCharacter::Elim(bool bPlayerLeftGame)
{
	MulticastElim(bPlayerLeftGame);
	DropOrDestroyWeapons();
}
void AShooterCharacter::DropOrDestroyWeapons()
{
	if(Combat)
	{
		if(Combat->EquippedWeapon)
		{
			DropOrDestroyWeapon(Combat->EquippedWeapon);
		}
		if(Combat->SecondaryWeapon)
		{
			DropOrDestroyWeapon(Combat->SecondaryWeapon);
		}
		if(Combat->TheFlag)
		{
			Combat->TheFlag->Dropped();
		}
	}
}
void AShooterCharacter::SetSpawnPoint()
{
	if(HasAuthority() && ShooterPlayerState->GetTeam() != ETeam::ET_NoTeam)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this,ATeamPlayerStart::StaticClass(),PlayerStarts);
		TArray<ATeamPlayerStart*> TeamPlayerStarts;
		for(auto Start : PlayerStarts)
		{
			ATeamPlayerStart* TeamStart = Cast<ATeamPlayerStart>(Start);
			if(TeamStart && TeamStart->Team == ShooterPlayerState->GetTeam())
			{
				TeamPlayerStarts.Add(TeamStart);
			}
		}
		if(TeamPlayerStarts.Num() > 0)
		{
			ATeamPlayerStart* ChosenPlayerStart = TeamPlayerStarts[FMath::RandRange(0,TeamPlayerStarts.Num() - 1 )];
			SetActorLocationAndRotation(ChosenPlayerStart->GetActorLocation(),ChosenPlayerStart->GetActorRotation());
		}
	}
}
void AShooterCharacter::OnPlayerStateInitialized()
{
	ShooterPlayerState->AddToScore(0.f);
	ShooterPlayerState->AddToDefeats(0);
	SetTeamColor(ShooterPlayerState->GetTeam());
	SetSpawnPoint();
}
void AShooterCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if(Weapon == nullptr) return;
	if(Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}
}
void AShooterCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
	if(ShooterPlayerController)
	{
		ShooterPlayerController->SetHudWeaponAmmo(0);
	}
	PlayElimMontage();
	bIsElimmed = true;
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
	/*if(ShooterPlayerController)
	{
		ShooterPlayerController->bDisableGameplay = true;
	}*/
	if(Combat)
	{
		Combat->FireButtonPressed(false);
	}
	//Disable Collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if(IsLocallyControlled() && Combat && Combat->bAiming && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Sniper)
	{
		ShowSniperScopeWidget(false);
	}
	if(CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
	GetWorldTimerManager().SetTimer(ElimTimer,this,&AShooterCharacter::ElimTimerFinished,ElimDelay);
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
	ShooterGameMode =  ShooterGameMode == nullptr ?  GetWorld()->GetAuthGameMode<AShooterGameModeBase>() : ShooterGameMode;
	if(ShooterGameMode && !bLeftGame)
	{
		ShooterGameMode->RequestRespawn(this,Controller);
	}
	if(bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}

}
void AShooterCharacter::ServerLeaveGame_Implementation()
{
	ShooterGameMode =  ShooterGameMode == nullptr ?  GetWorld()->GetAuthGameMode<AShooterGameModeBase>() : ShooterGameMode;

	ShooterPlayerState = ShooterPlayerState == nullptr ? GetPlayerState<AShooterPlayerState>() : ShooterPlayerState;
	if(ShooterGameMode && ShooterPlayerState)
	{
		ShooterGameMode->PlayerLeftGame(ShooterPlayerState);
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

	RotateInPlace(DeltaTime);
	HideCamera();
	PollInit();
	
}
void AShooterCharacter::RotateInPlace(float DeltaTime)
{
	if(Combat && Combat->bHoldingFlag)
	{
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		TurningInPlace = ETurnInPlace::ETurnIP_NotTurning;
		return;
	}
	if(Combat && Combat->EquippedWeapon)
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = true;
	}
	if(ShooterPlayerController)
	{
		if(ShooterPlayerController->bDisableGameplay)
		{
			bUseControllerRotationYaw = false;
			TurningInPlace = ETurnInPlace::ETurnIP_NotTurning;
			return;
		}
	}
	AimOffset(DeltaTime);
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
	ShooterGameMode =  ShooterGameMode == nullptr ?  GetWorld()->GetAuthGameMode<AShooterGameModeBase>() : ShooterGameMode;
	if(bIsElimmed || ShooterGameMode == nullptr) return;
	Damage = ShooterGameMode->CalculateDamage(InstigatorController,Controller,Damage);


	float DamageToHealth = Damage;
	if(Shield > 0.f)
	{
		if(Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - Damage,0.f,MaxShield);
			DamageToHealth = 0.f;
		}
		else
		{
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield,0.f,Damage);
			Shield = 0.f;
		}
	}
	Health = FMath::Clamp(Health - DamageToHealth,0.f,MaxHealth);
	UpdateHudHealth();
	UpdateHudShield();
	PlayHitReactMontage();

	if(Health  == 0.f)
	{
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
			OnPlayerStateInitialized();
			AShooterGameState* ShooterGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
			if(ShooterGameState && ShooterGameState->TopScoringPlayer.Contains(ShooterPlayerState))
			{
				MulticastGainedTheLead();
			}
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
	if(Buff)
	{
		Buff->Character = this;
		Buff->SetInitialSpeed(GetShooterCharacterComponent()->MaxWalkSpeed,GetShooterCharacterComponent()->MaxWalkSpeedCrouched);
		Buff->SetInitialJumpVelocity(GetShooterCharacterComponent()->JumpZVelocity);
	}
	if(LagCompensation)
	{
		LagCompensation->Character = this;
		if(Controller)
		{
			LagCompensation->Controller = Cast<AShooterPlayerController>(Controller);
		}
	}
}

void AShooterCharacter::Destroyed()
{
	Super::Destroyed();
	
	ShooterGameMode =  ShooterGameMode == nullptr ?  GetWorld()->GetAuthGameMode<AShooterGameModeBase>() : ShooterGameMode;
	bool bMatchNotInProgress = ShooterGameMode && ShooterGameMode->GetMatchState() != MatchState::InProgress;
	
	if(Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy();
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
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SMG:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWT_Sniper:
			SectionName = FName("Sniper");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("Grenade");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
	
}
void AShooterCharacter::PlaySwapMontage()
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if(AnimInstance && Swap)
	{
		AnimInstance->Montage_Play(Swap);
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
void AShooterCharacter::PlayThrowGrenadeMontage() const
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if(AnimInstance && GrenadeToss)
	{
		AnimInstance->Montage_Play(GrenadeToss);
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

bool AShooterCharacter::IsCrouching()
{
	if(GetShooterCharacterComponent()->bWantsToCrouch)
	{
		return true;
	}
	return false;
}
bool AShooterCharacter::IsSliding()
{
	if(GetShooterCharacterComponent()->IsSliding())
	{
		return true;
	}
	return false;
}

 void AShooterCharacter::SpawnDefaultWeapon()
{
	ShooterGameMode =  ShooterGameMode == nullptr ?  GetWorld()->GetAuthGameMode<AShooterGameModeBase>() : ShooterGameMode;
	UWorld* World = GetWorld();
	if(ShooterGameMode && World && !bIsElimmed && DefaultWeaponClass)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;
		if(Combat)
		{
			Combat->EquipWeapon(StartingWeapon);
		}
	}
}

 void AShooterCharacter::Jump()
 {
	 Super::Jump();
	bPressedShooterJump = true;
	bPressedJump = false;
 }

 void AShooterCharacter::StopJumping()
 {
	 Super::StopJumping();
	bPressedShooterJump = false;
 }

void AShooterCharacter::SetTeamColor(ETeam Team)
{
	if(GetMesh() == nullptr || OriginalMaterial == nullptr) return;
	switch (Team)
	{
	case ETeam::ET_NoTeam:
		GetMesh()->SetMaterial(0,OriginalMaterial);
		DissolveMaterialInstance = BlueDissolveMaterialInstance;
		break;
	case ETeam::ET_BlueTeam:
		GetMesh()->SetMaterial(0,BlueMaterial);
		DissolveMaterialInstance = BlueDissolveMaterialInstance;
		break;
	case ETeam::ET_RedTeam:
		GetMesh()->SetMaterial(0,RedMaterial);
		DissolveMaterialInstance = RedDissolveMaterialInstance;
		break;
	}
}
void AShooterCharacter::EquipButtonPressed()
{
	if(Combat)
	{
		if(Combat->bHoldingFlag) return;
		if(Combat->CombatState == ECombatState::ECState_Unoccupied) ServerEquipButtonPressed();
		if(Combat->bShouldSwapWeapon() && !HasAuthority() && Combat->CombatState == ECombatState::ECState_Unoccupied && OverlappingWeapon == nullptr)
		{
			PlaySwapMontage();
			Combat->CombatState = ECombatState::ECState_SwapWeapons;
			bFinishedSwapping = false;
		}
	}
}
void AShooterCharacter::CrouchButtonPressed()
{
	if(Combat && Combat->bHoldingFlag) return;
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
		if(Combat->bHoldingFlag) return;
		Combat->SetAiming(true);
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Holding!"));	
	}
}
void AShooterCharacter::AimButtonReleased()
{
	if(Combat)
	{
		if(Combat->bHoldingFlag) return;
		Combat->SetAiming(false);
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Released!"));	
	}
}
void AShooterCharacter::FireButtonPressed()
{
	if(Combat)
	{
		if(Combat->bHoldingFlag) return;
		Combat->FireButtonPressed(true);
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Firing!"));	
	}
}
void AShooterCharacter::FireButtonReleased()
{
	if(Combat)
	{
		if(Combat->bHoldingFlag) return;
		Combat->FireButtonPressed(false);
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Stopped!"));	

	}
}
void AShooterCharacter::ReloadButtonPressed()
{
	if(Combat)
	{
		if(Combat->bHoldingFlag) return;
		Combat->Reload();
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Reloading!"));	
	}
}
void AShooterCharacter::GrenadeButtonPressed()
{
	if(Combat)
	{
		if(Combat->bHoldingFlag) return;
		Combat->ThrowGrenade();
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("LEEEEEROYYYYYYY!"));	
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
	UE_LOG(LogTemp,Warning,TEXT("AO_YAW : %f"),AO_Yaw);
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
		if(Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if(Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
		if(Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}
void AShooterCharacter::OnRep_Health(float LastHealth)
{
	UpdateHudHealth();
	if(Health < LastHealth)
	{
		PlayHitReactMontage();
	}
	
}
void AShooterCharacter::OnRep_Shield(float LastShield)
{
	UpdateHudShield();
	if(Shield < LastShield)
	{
		PlayHitReactMontage();
	}
}
void AShooterCharacter::ServerEquipButtonPressed_Implementation()
{
	if(Combat)
	{
		if(OverlappingWeapon)
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else if (Combat->bShouldSwapWeapon())
		{
			Combat->SwapWeapon();
		}
	}
}
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
bool AShooterCharacter::IsHoldingFlag() const
{
	if(Combat == nullptr) return false;
	return Combat->bHoldingFlag;
}
void AShooterCharacter::SetHoldingFlag(bool bHolding)
{
	if(Combat == nullptr) return;
	Combat->bHoldingFlag = bHolding;
}
ECombatState AShooterCharacter::GetCombatState() const
{
	if(Combat == nullptr) return ECombatState::ECState_MAX;
	return Combat->CombatState;
}
bool AShooterCharacter::IsLocallyReloading()
{
	if(Combat == nullptr) return false;
	return Combat->bLocallyReloading;
}
ETeam AShooterCharacter::GetTeam()
{
	ShooterPlayerState = ShooterPlayerState == nullptr ? GetPlayerState<AShooterPlayerState>() : ShooterPlayerState;
	if(ShooterPlayerState == nullptr) return ETeam::ET_NoTeam;
	return ShooterPlayerState->GetTeam();
}

