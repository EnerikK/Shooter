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
	if(HasAuthority())
	{
		PickUpSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		PickUpSphere->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
		PickUpSphere->OnComponentBeginOverlap.AddDynamic(this,&AWeapon::OnSphereOverlap);
		PickUpSphere->OnComponentEndOverlap.AddDynamic(this,&AWeapon::OnSphereEndOverlap);
	}
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
	DOREPLIFETIME(AWeapon,Ammo);
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
	if(HasAuthority())
	{
		SpendRound();
	}
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
void AWeapon::OnRep_Ammo()
{
	ShooterOwnerCharacter = ShooterOwnerCharacter == nullptr ? Cast<AShooterCharacter>(GetOwner()) : ShooterOwnerCharacter;
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
void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1,0,MagCapacity);
	SetHudAmmo();
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
void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo - AmmoToAdd,0,MagCapacity);
	SetHudAmmo();
}
void AWeapon::ShowPickUpWidget(bool bShowWidget)
{
	if(PickUpWidget)
	{
		PickUpWidget->SetVisibility(bShowWidget);
	}
}

