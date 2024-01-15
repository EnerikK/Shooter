// Hello :) 


#include "Components/CombatComponent.h"
#include "Character/ShooterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"
#include "DrawDebugHelpers.h"
#include "Hud/ShooterHUD.h"
#include "Player/ShooterPlayerController.h"

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
	}
}
void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
 	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
 
 	DOREPLIFETIME(UCombatComponent,EquippedWeapon);
 	DOREPLIFETIME(UCombatComponent,bAiming);
}
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
 	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	SetHudCrosshair(DeltaTime);
	
	/*FHitResult HitResult;
	TraceUnderCrosshair(HitResult);*/
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
			FHUDPackage HUDPackage;
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
			
			HUDPackage.CrosshairSpread = CrosshairVelocity + CrosshairInAir;
			Hud->SetHudPackage(HUDPackage);
		}
	}
	
}
void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
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
void UCombatComponent::OnRep_EquippedWeapon()
{
	if(EquippedWeapon && Character)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}
void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if(bFireButtonPressed)
	{
		FHitResult HitResult;
		TraceUnderCrosshair(HitResult);
		ServerFire(HitResult.ImpactPoint);
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
		FVector End = Start + CrossHairWorldDirection * TRACE;
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECC_Visibility
		);
		if (!TraceHitResult.bBlockingHit) { TraceHitResult.ImpactPoint = End; }
	}
}
void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if(EquippedWeapon == nullptr) return;
	if(Character)
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

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EW_Equipped);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("Hand_R_Socket"));
	if(HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon,Character->GetMesh());
	}
	EquippedWeapon->SetOwner(Character);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
	
}

