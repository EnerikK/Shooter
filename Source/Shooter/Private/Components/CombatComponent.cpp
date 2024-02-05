// Hello :) 


#include "Components/CombatComponent.h"
#include "Character/ShooterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"
#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "Hud/ShooterHUD.h"
#include "Player/ShooterPlayerController.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Weapon/Projectile.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 470.f;
	
}
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed=BaseWalkSpeed;
		if(Character->GetFollowCamera())
		{
			DefaultPov = Character->GetFollowCamera()->FieldOfView;
			CurrentPov = DefaultPov;
		}
		if(Character->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}
}
void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
 	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
 
 	DOREPLIFETIME(UCombatComponent,EquippedWeapon);
 	DOREPLIFETIME(UCombatComponent,bAiming);
	DOREPLIFETIME(UCombatComponent,CombatState);
	DOREPLIFETIME_CONDITION(UCombatComponent,CarriedAmmo,COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent,Grenades);

	
}
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
 	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if(Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshair(HitResult);
		HitTarget = HitResult.ImpactPoint;
		
		SetHudCrosshair(DeltaTime);
		InterpPov(DeltaTime);
	}
	/*FHitResult HitResult;
	TraceUnderCrosshair(HitResult);*/
}
void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if(bFireButtonPressed)
	{
		Fire();
	}
	else
	{
		bCanFire = false;
	}
}
void UCombatComponent::ShotgunShellReload()
{
	if(Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoValues();
	}
}
void UCombatComponent::Fire()
{
	if(CanFire())
	{
		bCanFire = false;
		ServerFire(HitTarget);
		if(EquippedWeapon)
		{
			CrosshairShooting = 0.75f;
		}
		StartFiretimer();
	}
}
void UCombatComponent::StartFiretimer()
{
	if(EquippedWeapon == nullptr || Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(Firetimer,this,&UCombatComponent::FireTimerFinished,EquippedWeapon->FireDelay);
}
void UCombatComponent::FireTimerFinished()
{
	if(EquippedWeapon == nullptr ) return;
	bCanFire = true;
	if(bFireButtonPressed && EquippedWeapon->bIsAutomatic)
	{
		Fire();
	}
	ReloadEmptyWeapon();
}
bool UCombatComponent::CanFire()
{
	if(EquippedWeapon == nullptr) return false;
	if(!EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECState_Reloading &&
	EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun) return true;
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECState_Unoccupied;
	
}
void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHudCarriedAmmo(CarriedAmmo);
	}
	bool bJumpToShotgunEnd = CombatState == ECombatState::ECState_Reloading &&
		EquippedWeapon != nullptr &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun &&
		CarriedAmmo == 0;
	if(bJumpToShotgunEnd)
	{
		JumpToShotgunEnd();
	}
}
void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssualtRifle,StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher,StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol,StatingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SMG,StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun,StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Sniper,StartingSniperAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher,StartingGrenadeLauncherAmmo);
}
void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if(EquippedWeapon == nullptr) return;
	if(Character && CombatState == ECombatState::ECState_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
		CombatState = ECombatState::ECState_Unoccupied;
		return;
	}
	if(Character && CombatState == ECombatState::ECState_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}
void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if(Character == nullptr || WeaponToEquip == nullptr) return;
	if(CombatState != ECombatState::ECState_Unoccupied) return;
	
	DropEquippedWeapon();
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EW_Equipped);
	
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHudAmmo();

	UpdateCarriedAmmo();
	PlayEquippedSound();
	ReloadEmptyWeapon();
	
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
	
}
void UCombatComponent::DropEquippedWeapon()
{
	if(EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
}
void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if(Character == nullptr || ActorToAttach == nullptr || Character->GetMesh() == nullptr || EquippedWeapon == nullptr) return;
	/*bool bUseRocketSocket = EquippedWeapon->GetWeaponType() == EWeaponType::EWT_RocketLauncher;
	FName SocketName = bUseRocketSocket ? FName("Hand_R_SocketRocket") : FName("Hand_R_Socket");*/ // Using a specific socket for a specific gun if need 
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("Hand_R_Socket"));
	if(HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach,Character->GetMesh());
	}
}
void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if(Character == nullptr || ActorToAttach == nullptr || Character->GetMesh() == nullptr) return;
	
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("Hand_L_Socket"));
	if(HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach,Character->GetMesh());
	}
}
void UCombatComponent::UpdateCarriedAmmo()
{
	if(EquippedWeapon == nullptr) return;
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	
	Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHudCarriedAmmo(CarriedAmmo);
	}
}
void UCombatComponent::PlayEquippedSound()
{
	if(Character && EquippedWeapon && EquippedWeapon->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this,EquippedWeapon->EquipSound,Character->GetActorLocation());
	}
}
void UCombatComponent::ReloadEmptyWeapon()
{
	if(EquippedWeapon && EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}
void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if(Character && Character->GetAttachedGrenade())
	{
		Character->GetAttachedGrenade()->SetVisibility(bShowGrenade);
	}
}
void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECState_Reloading:
		HandleReload();
		break;
	case ECombatState::ECState_Unoccupied:
		/*if(bFireButtonPressed)
		{
			Fire();
		}*/
		break;
	case ECombatState::ECState_ThrowGrenade:
		if(Character && !Character->IsLocallyControlled())
		{
			Character->PlayThrowGrenadeMontage();
			AttachActorToLeftHand(EquippedWeapon);
			ShowAttachedGrenade(true);
		}
		break;
	}
	
}
void UCombatComponent::Reload()
{
	if(CarriedAmmo > 0 && CombatState == ECombatState::ECState_Unoccupied && EquippedWeapon && !EquippedWeapon->IsFull())
	{
		ServerReload();
	}
	
}
void UCombatComponent::FinishReloading()
{
	if(Character == nullptr)return;
	if(Character->HasAuthority())
	{
		CombatState = ECombatState::ECState_Unoccupied;
		UpdateAmmoValues();
	}
	/*if(bFireButtonPressed)
	{
		Fire();
	}*/
}
void UCombatComponent::ServerReload_Implementation()
{
	if(Character == nullptr || EquippedWeapon == nullptr) return;
	
	CombatState = ECombatState::ECState_Reloading;
	HandleReload();
	
}
void UCombatComponent::HandleReload()
{
	Character->PlayReloadMontage();
}
int32 UCombatComponent::AmountToReload()
{
	if(EquippedWeapon == nullptr) return 0;
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();

	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag,AmountCarried);
		return FMath::Clamp(RoomInMag,0,Least);
	}
	return 0;
}
void UCombatComponent::ThrowGrenade()
{
	if(Grenades == 0) return;
	if(CombatState != ECombatState::ECState_Unoccupied && EquippedWeapon == nullptr) return;
	CombatState = ECombatState::ECState_ThrowGrenade;
	if(Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	if(Character && !Character->HasAuthority())
	{
		ServerThrowGrenade();
	}
	if(Character && Character->HasAuthority())
	{
		Grenades = FMath::Clamp(Grenades - 1 ,0 ,MaxGrenades);
		UpdateHudGrenades();
	}
}
void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if(Grenades == 0) return;
	CombatState = ECombatState::ECState_ThrowGrenade;
	if(Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	Grenades = FMath::Clamp(Grenades -1 ,0 ,MaxGrenades);
	UpdateHudGrenades();
}
void UCombatComponent::ThrowGrenadeFinish()
{
	CombatState = ECombatState::ECState_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
	
}
void UCombatComponent::LaunchGrenade()
{
	ShowAttachedGrenade(false);
	if(Character && Character->IsLocallyControlled())
	{
		ServerLaunchGrenade(HitTarget);
	}
}
void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if(Character && GrenadeClass && Character->GetAttachedGrenade())
	{
		const FVector StartingLoc = Character->GetAttachedGrenade()->GetComponentLocation();
		FVector ToTarget = Target - StartingLoc;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		UWorld* World = GetWorld();
		if(World)
		{
			World->SpawnActor<AProjectile>(GrenadeClass,StartingLoc,ToTarget.Rotation(),SpawnParams);
		}
	}
}
void UCombatComponent::UpdateAmmoValues()
{
	if(Character == nullptr || EquippedWeapon == nullptr) return;
	int32 ReloadAmount = AmountToReload();
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	
	Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHudCarriedAmmo(CarriedAmmo);
	}
	
	EquippedWeapon->AddAmmo(-ReloadAmount);
	
}
void UCombatComponent::UpdateShotgunAmmoValues()
{
	if(Character == nullptr || EquippedWeapon == nullptr) return;
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHudCarriedAmmo(CarriedAmmo);
	}
	EquippedWeapon->AddAmmo(-1);
	if(EquippedWeapon->IsFull() || CarriedAmmo == 0) //Go to shotgun end Section
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::OnRep_Grenades()
{
	UpdateHudGrenades();
}
void UCombatComponent::UpdateHudGrenades()
{
	Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller; //Setting Controller variable and if
	if(Controller)
	{
		Controller->SetHudGrenades(Grenades);
	}
}
void UCombatComponent::JumpToShotgunEnd()
{
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if(AnimInstance && Character->GetReloadMontage())
	{
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
	
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if(EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EW_Equipped);
		AttachActorToRightHand(EquippedWeapon);
		
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		
		PlayEquippedSound();
	}
}
void UCombatComponent::TraceUnderCrosshair(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	FVector2D CrosshairLocation(ViewportSize.X/2.f,ViewportSize.Y/2.f);//Center of the viewport
	FVector CrossHairWorldPosition;
	FVector CrossHairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this,0),
		CrosshairLocation,CrossHairWorldPosition,CrossHairWorldDirection
		);
	if(bScreenToWorld)
	{
		FVector Start = CrossHairWorldPosition;

		if(Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrossHairWorldDirection * (DistanceToCharacter + 100.f);
			DrawDebugSphere(GetWorld(),Start,16.f,12,FColor::Red,false);
		}
		
		FVector End = Start + CrossHairWorldDirection * TRACE;
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECC_Visibility
		);
		if (!TraceHitResult.bBlockingHit) { TraceHitResult.ImpactPoint = End; }
		if(TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractInterface>())
		{
			HUDPackage.CrosshairColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairColor = FLinearColor::White;
		}
	}
}
void UCombatComponent::SetHudCrosshair(float DeltaTime)
{
	if(Character == nullptr || Character->Controller == nullptr) return;

	Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller; //Setting Controller variable and if
	//its set we just set the controller to itself and we dont have to to do the  cast again 
	
	if(Controller)
	{
		Hud = Hud == nullptr ? Cast<AShooterHUD>(Controller->GetHUD()) : Hud; //same
		if(Hud)
		{
			if(EquippedWeapon)
			{
				HUDPackage.CrosshairCenter = EquippedWeapon->CrosshairCenter;
				HUDPackage.CrosshairLeft = EquippedWeapon->CrosshairLeft;
				HUDPackage.CrosshairRight = EquippedWeapon->CrosshairRight;
				HUDPackage.CrosshairBottom = EquippedWeapon->CrosshairBottom;
				HUDPackage.CrosshairTop = EquippedWeapon->CrosshairTop;
			}
			else
			{
				HUDPackage.CrosshairCenter = nullptr;
				HUDPackage.CrosshairLeft = nullptr;
				HUDPackage.CrosshairRight = nullptr;
				HUDPackage.CrosshairBottom = nullptr;
				HUDPackage.CrosshairTop = nullptr;
			}
			//Calculate crosshair spread
			//[0,600]->[0,1]
			FVector2d WalkSpeed(0.f,Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2d SpeedMultiplayer(0.f,1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;
			CrosshairVelocity = FMath::GetMappedRangeValueClamped(WalkSpeed,SpeedMultiplayer,Velocity.Size());

			if(Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAir = FMath::FInterpTo(CrosshairInAir,2.25f,DeltaTime,2.25f);
			}
			else
			{
				CrosshairInAir = FMath::FInterpTo(CrosshairInAir,0.f,DeltaTime,30.f);
			}
			if(bAiming)
			{
				CrosshairAim = FMath::FInterpTo(CrosshairAim,0.58f,DeltaTime,30.f);
			}
			else
			{
				CrosshairAim = FMath::FInterpTo(CrosshairAim,0.f,DeltaTime,30.f);
				
			}

			CrosshairShooting = FMath::FInterpTo(CrosshairShooting,0.f,DeltaTime,40.f);
			
			HUDPackage.CrosshairSpread = 0.5f + CrosshairVelocity + CrosshairInAir - CrosshairAim + CrosshairShooting;
			Hud->SetHudPackage(HUDPackage);
		}
	}
}
void UCombatComponent::InterpPov(float DeltaTime)
{
	if(EquippedWeapon == nullptr) return;
	if(bAiming)
	{
		CurrentPov = FMath::FInterpTo(CurrentPov,EquippedWeapon->GetZoomedPov(),DeltaTime,EquippedWeapon->GetZoomInterpPov());
	}
	else
	{
		CurrentPov = FMath::FInterpTo(CurrentPov,DefaultPov,DeltaTime,ZoomInterpSpeed);
	}
	if(Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentPov);
	}
}
void UCombatComponent::SetAiming(bool bIsAiming)
{
	if(Character == nullptr || EquippedWeapon == nullptr) return;
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
	if(Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Sniper)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
	}
}
void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}
