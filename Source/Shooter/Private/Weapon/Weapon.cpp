// Hello :) 


#include "Weapon/Weapon.h"
#include "Character/ShooterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Animation/AnimationAsset.h"
#include "Net/UnrealNetwork.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapon/AmmoEject.h"
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

	PickUpSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickUpSphere"));
	PickUpSphere->SetupAttachment(RootComponent);
	PickUpSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickUpSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickUpWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickUpWidget"));
	PickUpWidget->SetupAttachment(RootComponent);
	
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
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	switch (WeaponState)
	{
	case EWeaponState::EW_Equipped:
		ShowPickUpWidget(false);
		GetPickUpSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	}
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
void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
		case EWeaponState::EW_Equipped:
			ShowPickUpWidget(false);
			/*PickUpSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);*/
		break;
		
	}
	
}

void AWeapon::ShowPickUpWidget(bool bShowWidget)
{
	if(PickUpWidget)
	{
		PickUpWidget->SetVisibility(bShowWidget);
	}
}

