// Hello :) 


#include "Weapon/Shotgun.h"

#include "Character/ShooterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


void AShotgun::Fire(const FVector& HitTarget)
{
	AWeapon::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		
		TMap<AShooterCharacter*,uint32> HitMap;
		for(uint32 i = 0; i < NumberOfPellets; i++)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start,HitTarget,FireHit);
			AShooterCharacter* Character = Cast<AShooterCharacter>(FireHit.GetActor());
			if(Character && HasAuthority() && InstigatorController)
			{
				if(HitMap.Contains(Character))
				{
					HitMap[Character]++;
				}
				else
				{
					HitMap.Emplace(Character,1);
				}
			}
			if(ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),ImpactParticles,FireHit.ImpactPoint,FireHit.ImpactNormal.Rotation());
			}
			if(HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this,HitSound,FireHit.ImpactPoint,
					.5f,FMath::FRandRange(-.5f,.5f));
			}
		}
		for(auto HitPair : HitMap)
		{
			if(InstigatorController)
			{
				if(HitPair.Key && HasAuthority() && InstigatorController)
				{
					UGameplayStatics::ApplyDamage(HitPair.Key,Damage * HitPair.Value,InstigatorController,this,UDamageType::StaticClass());
				}
			}
		}
	}
}