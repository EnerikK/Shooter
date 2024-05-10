// Hello :) 


#include "Character/ShooterMovementComponent.h"

#include "Character/ShooterCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"

UShooterMovementComponent::UShooterMovementComponent()
{
	NavAgentProps.bCanCrouch = true;

}
void UShooterMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UShooterMovementComponent,Proxy_bSlideStart,COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UShooterMovementComponent,Proxy_bDashStart,COND_SkipOwner);


}

void UShooterMovementComponent::SprintPressed()
{
	bWantsToSprint = true;
}

void UShooterMovementComponent::SprintReleased()
{
	bWantsToSprint = false;
}

void UShooterMovementComponent::CrouchPressed()
{
	bWantsToCrouch = !bWantsToCrouch;
}

void UShooterMovementComponent::SlidePressed()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if(CurrentTime - SlideStartTime >= Slide_CooldownDuration)
	{
		bWantsToSlide = true;
		//SetCollisionSizeToSliding(SlideHalfHeight);
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer
		(TimerHandle_SlideCooldown,this,
		&UShooterMovementComponent::OnSlideCooldownFinished,Slide_CooldownDuration - (CurrentTime - SlideStartTime));
	}
}

void UShooterMovementComponent::SlideReleased()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_SlideCooldown);
	bWantsToSlide = false;
}

void UShooterMovementComponent::DashPressed()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if(CurrentTime - DashStartTime >= DashCooldown)
	{
		bWantsToDash = true;
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_DashCooldown,this,&UShooterMovementComponent::OnDashCooldownFinished,DashCooldown - (CurrentTime - DashStartTime));
	}
}

void UShooterMovementComponent::DashReleased()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_DashCooldown);
	bWantsToDash = false;
}

bool UShooterMovementComponent::IsCustomMovementMode(EMovementModeBattleMage InCustomMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}
void UShooterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
	
	PlayerCharacter = Cast<AShooterCharacter>(GetOwner());
}

bool UShooterMovementComponent::FSavedMove_ShooterCharacter::CanCombineWith(const FSavedMovePtr& NewMove,
	ACharacter* InCharacter, float MaxDelta) const
{
	const FSavedMove_ShooterCharacter* NewShooterMove = static_cast<FSavedMove_ShooterCharacter*>(NewMove.Get());
	if(Saved_bWantsToSprint != NewShooterMove->Saved_bWantsToSprint)
	{
		return false;
	}
	if(Saved_bWantsToSlide != NewShooterMove->Saved_bWantsToSlide)
	{
		return false;
	}
	if(Saved_bWantsToDash != NewShooterMove->Saved_bWantsToDash)
	{
		return false;
	}
	
	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UShooterMovementComponent::FSavedMove_ShooterCharacter::Clear()
{
	FSavedMove_Character::Clear();
	FSavedMove_Character::Clear();
	
	Saved_bWantsToSprint = 0;
	Saved_bWantsToSlide = 0;
	Saved_bWantsToDash = 0;
	Saved_bHadAnimRootMotion = 0;
}

uint8 UShooterMovementComponent::FSavedMove_ShooterCharacter::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();
	if(Saved_bWantsToSprint)  Result |= FLAG_Custom_0;
	if(Saved_bWantsToSlide) Result |= FLAG_Custom_1;
	if(Saved_bWantsToDash) Result |= Flag_Dash;
	return  Result;
}

void UShooterMovementComponent::FSavedMove_ShooterCharacter::SetMoveFor(ACharacter* C, float InDeltaTime,
	FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	const UShooterMovementComponent* CharacterMovement = Cast<UShooterMovementComponent>(C->GetCharacterMovement());
	Saved_bWantsToSprint = CharacterMovement->bWantsToSprint;
	Saved_bWantsToSlide = CharacterMovement->bWantsToSlide;
	Saved_bHadAnimRootMotion = CharacterMovement->bHadAnimRootMotion;
	Saved_bWantsToDash = CharacterMovement->bWantsToDash;
}

void UShooterMovementComponent::FSavedMove_ShooterCharacter::PrepMoveFor(ACharacter* C)
{
	FSavedMove_Character::PrepMoveFor(C);
	UShooterMovementComponent* CharacterMovement = Cast<UShooterMovementComponent>(C->GetCharacterMovement());
	CharacterMovement->bWantsToSprint = Saved_bWantsToSprint;
	CharacterMovement->bWantsToSlide = Saved_bWantsToSlide;
	CharacterMovement->bHadAnimRootMotion = Saved_bHadAnimRootMotion;
}

UShooterMovementComponent::FNetworkPredictionData_Client_ShooterCharacter::
FNetworkPredictionData_Client_ShooterCharacter(const UCharacterMovementComponent& ClientMovement) : Super(ClientMovement)
{
}

FSavedMovePtr UShooterMovementComponent::FNetworkPredictionData_Client_ShooterCharacter::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_ShooterCharacter());
}

FNetworkPredictionData_Client* UShooterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr)
	if(ClientPredictionData == nullptr)
	{
		UShooterMovementComponent* MutableThis = const_cast<UShooterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_ShooterCharacter(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}
	return ClientPredictionData;
}

bool UShooterMovementComponent::IsMovingOnGround() const
{
	return Super::IsMovingOnGround() || IsCustomMovementMode(BMove_Slide);
}

bool UShooterMovementComponent::CanCrouchInCurrentState() const
{
	return Super::CanCrouchInCurrentState() && IsMovingOnGround();
}

void UShooterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
	
	bWantsToSprint = (Flags & FSavedMove_ShooterCharacter::FLAG_Custom_0) != 0;
	
	bWantsToSlide = (Flags & FSavedMove_ShooterCharacter::FLAG_Custom_1) != 0;

	bWantsToDash = (Flags & FSavedMove_ShooterCharacter::Flag_Dash) != 0;
}

void UShooterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation,
	const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	if(MovementMode == MOVE_Walking)
	{
		if(bWantsToSprint)
		{
			MaxWalkSpeed = Sprint_MaxWalkSpeed;
		}
		else
		{
			MaxWalkSpeed = Walk_MaxWalkSpeed;
		}
	}
}

void UShooterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);
}

void UShooterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
	/*Slide*/
	
	bool bAuthProxy = CharacterOwner->HasAuthority() && !CharacterOwner->IsLocallyControlled();
	if(bWantsToSlide && CanSlide())
	{
		if(!bAuthProxy || GetWorld()->GetTimeSeconds() - SlideStartTime > AuthSlideCooldownDuration)
		{
			PerformSlide();
			bWantsToSlide = false;
			Proxy_bSlideStart = !Proxy_bSlideStart;
		}
		else
		{
			UE_LOG(LogTemp,Warning,TEXT("Client Tried to cheat"))
		}
	}
	if(IsSliding() && !bWantsToSlide)
	{
		SlideReleased();
	}
	
	/*Dash*/
	bool bAuthProxyDash = CharacterOwner->HasAuthority() && !CharacterOwner->IsLocallyControlled();
	if(bWantsToDash && CanDash())
	{
		if(bAuthProxyDash || GetWorld()->GetTimeSeconds() - DashStartTime > AuthDashCooldownDuration)
		{
			PerformDash();
			bWantsToDash = false;
			Proxy_bDashStart = !Proxy_bDashStart;
		}
		else
		{
			UE_LOG(LogTemp,Warning,TEXT("Cleint tried to dash"));
		}
	}
}

void UShooterMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateAfterMovement(DeltaSeconds);
}

void UShooterMovementComponent::SetCollisionSizeToSliding(float Size)
{
	const float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float OldUnscaledRadius = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();

	const float ClampedCrouchedHalfHeight = FMath::Max3(0.f,OldUnscaledRadius,Size);
	float HalfHeightAdjust = (OldUnscaledHalfHeight - ClampedCrouchedHalfHeight);
	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius,ClampedCrouchedHalfHeight);
	if (bCrouchMaintainsBaseLocation)
	{
		// Intentionally not using MoveUpdatedComponent, where a horizontal plane constraint would prevent the base of the capsule from staying at the same spot.
		UpdatedComponent->MoveComponent
			(FVector(0.f,0.f, -(HalfHeightAdjust * ComponentScale)),
			UpdatedComponent->GetComponentQuat(),
			true,
			nullptr, EMoveComponentFlags::MOVECOMP_NoFlags,
			ETeleportType::TeleportPhysics);
	}
	bForceNextFloorCheck = true;

	// OnStartCrouch takes the change from the Default size, not the current one (though they are usually the same).
	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	HalfHeightAdjust = (DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - ClampedCrouchedHalfHeight);

	AdjustProxyCapsuleSize();
	CharacterOwner->OnStartCrouch( HalfHeightAdjust, HalfHeightAdjust * ComponentScale);
}

bool UShooterMovementComponent::RestoreDefaultCollision()
{
	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	const float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float HalfHeightAdjust = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - OldUnscaledHalfHeight;
	const float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	const FVector PawnLocation = UpdatedComponent->GetComponentLocation();

	const UWorld* MyWorld = GetWorld();
	constexpr float SweepInflation = UE_KINDA_SMALL_NUMBER * 10.f;
	FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(CrouchTrace), false, CharacterOwner);
	FCollisionResponseParams ResponseParam;
	InitCollisionParams(CapsuleParams, ResponseParam);

	// Compensate for the difference between current capsule size and standing size
	const FCollisionShape StandingCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -SweepInflation - ScaledHalfHeightAdjust); // Shrink by negative amount, so actually grow it.
	const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
	bool bEncroached = true;

	if (!bCrouchMaintainsBaseLocation)
		{
			// Expand in place
			bEncroached = MyWorld->OverlapBlockingTestByChannel(PawnLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
		
			if (bEncroached)
			{
				// Try adjusting capsule position to see if we can avoid encroachment.
				if (ScaledHalfHeightAdjust > 0.f)
				{
					// Shrink to a short capsule, sweep down to base to find where that would hit something, and then try to stand up from there.
					float PawnRadius, PawnHalfHeight;
					CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);
					const float ShrinkHalfHeight = PawnHalfHeight - PawnRadius;
					const float TraceDist = PawnHalfHeight - ShrinkHalfHeight;
					const FVector Down = FVector(0.f, 0.f, -TraceDist);

					FHitResult Hit(1.f);
					const FCollisionShape ShortCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, ShrinkHalfHeight);
					const bool bBlockingHit = MyWorld->SweepSingleByChannel(Hit, PawnLocation, PawnLocation + Down, FQuat::Identity, CollisionChannel, ShortCapsuleShape, CapsuleParams);
					if (Hit.bStartPenetrating)
					{
						bEncroached = true;
					}
					else
					{
						// Compute where the base of the sweep ended up, and see if we can stand there
						const float DistanceToBase = (Hit.Time * TraceDist) + ShortCapsuleShape.Capsule.HalfHeight;
						const FVector NewLoc = FVector(PawnLocation.X, PawnLocation.Y, PawnLocation.Z - DistanceToBase + StandingCapsuleShape.Capsule.HalfHeight + SweepInflation + MIN_FLOOR_DIST / 2.f);
						bEncroached = MyWorld->OverlapBlockingTestByChannel(NewLoc, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
						if (!bEncroached)
						{
							// Intentionally not using MoveUpdatedComponent, where a horizontal plane constraint would prevent the base of the capsule from staying at the same spot.
							UpdatedComponent->MoveComponent(NewLoc - PawnLocation, UpdatedComponent->GetComponentQuat(), false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
						}
					}
				}
			}
		}
	if(bEncroached)
	{
		return false;
	}
	CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), true);

	const float MeshAdjust = ScaledHalfHeightAdjust;
	AdjustProxyCapsuleSize();
	CharacterOwner->OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	return true;
	
}

void UShooterMovementComponent::OnRep_SlideStart()
{
	PlayerCharacter->PlayAnimMontage(PlayerCharacter->GetSlideMontage());
	SlideStartDelegate.Broadcast();
}

void UShooterMovementComponent::OnSlideCooldownFinished()
{
	bWantsToSlide = true;
}

bool UShooterMovementComponent::CanSlide() const
{
	return IsWalking() && !IsCrouching();

}
void UShooterMovementComponent::PerformSlide()
{
	SlideStartTime = GetWorld()->GetTimeSeconds();
	FVector SlideDirection = (Acceleration.IsNearlyZero() ? UpdatedComponent->GetForwardVector() : Acceleration).GetSafeNormal2D();
	Velocity = Slide_EnterImpulse * SlideDirection;

	FQuat NewRotation = FRotationMatrix::MakeFromXZ(SlideDirection,FVector::UpVector).ToQuat();
	FHitResult Hit;
	SafeMoveUpdatedComponent(FVector::ZeroVector,NewRotation,false,Hit);
	SetMovementMode(MOVE_Walking);
	PlayerCharacter->PlayAnimMontage(PlayerCharacter->GetSlideMontage());
	SlideStartDelegate.Broadcast();
}

void UShooterMovementComponent::OnRep_DashStart()
{
	if(Proxy_bDashStart)
	{
		DashStartDelegate.Broadcast();
	}
}

bool UShooterMovementComponent::CanDash() const
{
	return IsWalking() && !IsCrouching();
}

void UShooterMovementComponent::PerformDash()
{
	DashStartTime = GetWorld()->GetTimeSeconds();
	FVector DashDirection = (Acceleration.IsNearlyZero() ? UpdatedComponent->GetForwardVector() : Acceleration).GetSafeNormal2D();
	DashDirection += FVector::UpVector * .1f;
	Velocity = DashImpulse * DashDirection;

	FQuat NewRotation = FRotationMatrix::MakeFromXZ(DashDirection,FVector::UpVector).ToQuat();
	FHitResult HitResult;
	SafeMoveUpdatedComponent(FVector::ZeroVector,NewRotation,false,HitResult);
	SetMovementMode(MOVE_Falling);
	PlayerCharacter->PlayAnimMontage(PlayerCharacter->GetDashMontage());
	DashStartDelegate.Broadcast();
}

void UShooterMovementComponent::OnDashCooldownFinished()
{
	bWantsToDash = true;
}
