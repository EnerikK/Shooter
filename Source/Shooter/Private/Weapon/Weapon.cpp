// Hello :) 


#include "Weapon/Weapon.h"
#include "Character/ShooterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Animation/AnimationAsset.h"
#include "Components/CombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/ShooterPlayerController.h"
#include "Weapon/AmmoEject.h"
#include "Weapon/WeaponTypes.h"
#include "Weapon/Projectile.h"


AWeapon::AWeapon()
{
 	
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh->SetCustomDepthStencilValue(250);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	PickUpSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickUpSphere"));
	PickUpSphere->SetupAttachment(RootComponent);
	PickUpSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickUpSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickUpWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickUpWidget"));
	PickUpWidget->SetupAttachment(RootComponent);
	
}

void AWeapon::EnableCustomDepth(bool bEnable)
{
	if(WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	PickUpSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PickUpSphere->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
	PickUpSphere->OnComponentBeginOverlap.AddDynamic(this,&AWeapon::OnSphereOverlap);
	PickUpSphere->OnComponentEndOverlap.AddDynamic(this,&AWeapon::OnSphereEndOverlap);
	if(PickUpWidget)
	{
		PickUpWidget->SetVisibility(false);
	}
}
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeapon,WeaponState);
	DOREPLIFETIME_CONDITION(AWeapon,bUseServerSideRewind,COND_OwnerOnly);
}

void AWeapon::OnPingTooHigh(bool bPingTooHigh)
{
	 bUseServerSideRewind = !bPingTooHigh;
	
}
void AWeapon::Fire(const FVector& HitTarget)
{
	if(FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation,false);
	}
	if(AmmoClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if(AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
			UWorld* World = GetWorld();
			if(World)
			{
				World->SpawnActor<AAmmoEject>(
					AmmoClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator()
				);
			}
		}
	}
	SpendRound();
	
}
FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(MuzzleFlashSocket == nullptr) return FVector();

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();
	
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f,SphereRadius);
	const FVector EndLoc = SphereCenter + RandVec;
	const FVector ToEndLoc = EndLoc - TraceStart;
		
	/*DrawDebugSphere(GetWorld(),SphereCenter,SphereRadius,12,FColor::Red,true);
	DrawDebugSphere(GetWorld(),EndLoc,4.f,12,FColor::Orange,true);
	DrawDebugLine(GetWorld(),TraceStart,FVector(TraceStart + ToEndLoc * TRACE / ToEndLoc.Size()),FColor::Black,true);*/

	return  FVector(TraceStart + ToEndLoc * TRACE / ToEndLoc.Size());
	
}
void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EW_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld,true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	ShooterOwnerCharacter = nullptr;
	ShooterOwnerPlayerController = nullptr;
	
}
void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	OnWeaponStateSet();
}
void AWeapon::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EW_Equipped:
		OnEquipped();
		break;
	case EWeaponState::EW_EquippedSecondary:
		OnEquippedSecondary();
		break;
	case EWeaponState::EW_Dropped:
		OnDropped();
		break;
	}
}
void AWeapon::OnRep_WeaponState()
{
	OnWeaponStateSet();

}
void AWeapon::OnEquipped()
{
	ShowPickUpWidget(false);
	GetPickUpSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if(WeaponType == EWeaponType::EWT_SMG)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
	EnableCustomDepth(false);
	ShooterOwnerCharacter = ShooterOwnerCharacter == nullptr ? Cast<AShooterCharacter>(GetOwner()) : ShooterOwnerCharacter;
	if(ShooterOwnerCharacter && bUseServerSideRewind)
	{
		ShooterOwnerPlayerController = ShooterOwnerPlayerController == nullptr ? Cast<AShooterPlayerController>(ShooterOwnerCharacter->Controller) : ShooterOwnerPlayerController;
		if(ShooterOwnerPlayerController && HasAuthority() && !ShooterOwnerPlayerController->HighPingDelegate.IsBound())
		{
			ShooterOwnerPlayerController->HighPingDelegate.AddDynamic(this,&AWeapon::OnPingTooHigh);
		}
	}
}
void AWeapon::OnDropped()
{
	if(HasAuthority())
	{
		GetPickUpSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
		
	WeaponMesh->SetCustomDepthStencilValue(251);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);
	
	ShooterOwnerCharacter = ShooterOwnerCharacter == nullptr ? Cast<AShooterCharacter>(GetOwner()) : ShooterOwnerCharacter;
	if(ShooterOwnerCharacter && bUseServerSideRewind)
	{
		ShooterOwnerPlayerController = ShooterOwnerPlayerController == nullptr ? Cast<AShooterPlayerController>(ShooterOwnerCharacter->Controller) : ShooterOwnerPlayerController;
		if(ShooterOwnerPlayerController && HasAuthority() && ShooterOwnerPlayerController->HighPingDelegate.IsBound())
		{
			ShooterOwnerPlayerController->HighPingDelegate.RemoveDynamic(this,&AWeapon::OnPingTooHigh);
		}
	}
}
void AWeapon::OnEquippedSecondary()
{
	ShowPickUpWidget(false);
	GetPickUpSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if(WeaponType == EWeaponType::EWT_SMG)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
	EnableCustomDepth(false);
	
	ShooterOwnerCharacter = ShooterOwnerCharacter == nullptr ? Cast<AShooterCharacter>(GetOwner()) : ShooterOwnerCharacter;
	if(ShooterOwnerCharacter && bUseServerSideRewind)
	{
		ShooterOwnerPlayerController = ShooterOwnerPlayerController == nullptr ? Cast<AShooterPlayerController>(ShooterOwnerCharacter->Controller) : ShooterOwnerPlayerController;
		if(ShooterOwnerPlayerController && HasAuthority() && ShooterOwnerPlayerController->HighPingDelegate.IsBound())
		{
			ShooterOwnerPlayerController->HighPingDelegate.RemoveDynamic(this,&AWeapon::OnPingTooHigh);
		}
	}
}
bool AWeapon::IsEmpty()
{
	return Ammo <= 0;
}
bool AWeapon::IsFull()
{
	return Ammo == MagCapacity;
}
void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AShooterCharacter* Character = Cast<AShooterCharacter>(OtherActor);
	if(Character)
	{
		Character->SetOverlappingWeapon(this);
	}
}
void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AShooterCharacter* Character = Cast<AShooterCharacter>(OtherActor);
	if(Character)
	{
		Character->SetOverlappingWeapon(nullptr);
	}
}
void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1,0,MagCapacity);
	SetHudAmmo();
	if(HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else
	{
		++Sequence;
	}
}
void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if(HasAuthority()) return;
	Ammo = ServerAmmo;
	--Sequence;
	Ammo -= Sequence;
	SetHudAmmo();
}
void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd,0,MagCapacity);
	SetHudAmmo();
	ClientAddAmmo(AmmoToAdd);
}
void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if(HasAuthority()) return;
	Ammo = FMath::Clamp(Ammo + AmmoToAdd,0,MagCapacity);
	ShooterOwnerCharacter = ShooterOwnerCharacter == nullptr ?  Cast<AShooterCharacter>(GetOwner()) : ShooterOwnerCharacter;
	if(ShooterOwnerCharacter && ShooterOwnerCharacter->GetCombat() && IsFull())
	{
		ShooterOwnerCharacter->GetCombat()->JumpToShotgunEnd();
	}
	SetHudAmmo();
}
void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if(Owner == nullptr)
	{
		ShooterOwnerCharacter = nullptr;
		ShooterOwnerPlayerController = nullptr;
	}
	else
	{
		ShooterOwnerCharacter = ShooterOwnerCharacter == nullptr ? Cast<AShooterCharacter>(Owner) : ShooterOwnerCharacter;
		if(ShooterOwnerCharacter && ShooterOwnerCharacter->GetEquippedWeapon() && ShooterOwnerCharacter->GetEquippedWeapon() == this)
		{
			SetHudAmmo();
		}
	
	}
}
void AWeapon::SetHudAmmo()
{
	ShooterOwnerCharacter = ShooterOwnerCharacter == nullptr ? Cast<AShooterCharacter>(GetOwner()) : ShooterOwnerCharacter;
	if(ShooterOwnerCharacter)
	{
		ShooterOwnerPlayerController = ShooterOwnerPlayerController == nullptr ? Cast<AShooterPlayerController>(ShooterOwnerCharacter->Controller) : ShooterOwnerPlayerController;
		if(ShooterOwnerPlayerController)
		{
			ShooterOwnerPlayerController->SetHudWeaponAmmo(Ammo);
		}
	}
}
void AWeapon::ShowPickUpWidget(bool bShowWidget)
{
	if(PickUpWidget)
	{
		PickUpWidget->SetVisibility(bShowWidget);
	}
}

