// Hello :) 


#include "Weapon/HitScanWeapon.h"

#include "Character/ShooterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(MuzzleFlashSocket && InstigatorController)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		FVector End = Start + (HitTarget - Start)* 1.25f;

		FHitResult FireHit;
		UWorld* World = GetWorld();
		if(World)
		{
			World->LineTraceSingleByChannel(FireHit,Start,End,ECC_Visibility);
			if(FireHit.bBlockingHit)
			{
				AShooterCharacter* Character = Cast<AShooterCharacter>(FireHit.GetActor());
				if(Character)
				{
					if(HasAuthority())
					{
						UGameplayStatics::ApplyDamage(Character,Damage,InstigatorController,this,UDamageType::StaticClass());
					}
				}
				if(ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(World,ImpactParticles,End,FireHit.ImpactNormal.Rotation());
				}
			}
		}
		
	}
	
}
