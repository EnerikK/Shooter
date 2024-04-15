// Hello :) 


#include "Weapon/Flag.h"
#include "Character/ShooterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"

AFlag::AFlag()
{
	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagMesh"));
	FlagMesh->SetIsReplicated(true);
	SetRootComponent(FlagMesh);
	GetPickUpSphere()->SetupAttachment(FlagMesh);
	GetPickUpWidget()->SetupAttachment(FlagMesh);
	FlagMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void AFlag::BeginPlay()
{
	Super::BeginPlay();
	InitialTransform = GetActorTransform();
}

void AFlag::Dropped()
{
	SetWeaponState(EWeaponState::EW_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld,true);
	FlagMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	ShooterOwnerCharacter = nullptr;
	ShooterOwnerPlayerController = nullptr;
	
}
void AFlag::ResetFlag()
{
	AShooterCharacter* FlagBearer = Cast<AShooterCharacter>(GetOwner());
	if(FlagBearer)
	{
		FlagBearer->SetHoldingFlag(false);
		FlagBearer->SetOverlappingWeapon(nullptr);
		FlagBearer->UnCrouch(); 
	}
	if(!HasAuthority()) return;
	
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld,true);
	FlagMesh->DetachFromComponent(DetachRules);
	SetWeaponState(EWeaponState::EW_Initial);
	GetPickUpSphere()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetPickUpSphere()->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
	
	SetOwner(nullptr);
	ShooterOwnerCharacter = nullptr;
	ShooterOwnerPlayerController = nullptr;
	SetActorTransform(InitialTransform);
}
void AFlag::OnEquipped()
{
	ShowPickUpWidget(false);
	GetPickUpSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	FlagMesh->SetSimulatePhysics(false);
	FlagMesh->SetEnableGravity(false);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FlagMesh->SetCollisionResponseToChannel(ECC_WorldDynamic,ECR_Overlap);
}
void AFlag::OnDropped()
{
	if(HasAuthority())
	{
		GetPickUpSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	FlagMesh->SetSimulatePhysics(true);
	FlagMesh->SetEnableGravity(true);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	FlagMesh->SetCollisionResponseToAllChannels(ECR_Block);
	FlagMesh->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);
	FlagMesh->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	
}

