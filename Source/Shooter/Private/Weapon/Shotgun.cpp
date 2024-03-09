// Hello :) 


#include "Weapon/Shotgun.h"
#include "Character/ShooterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/ShooterPlayerController.h"
#include "Sound/SoundCue.h"


void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();
		
		/*Maps hit character to number of time hits*/
		TMap<AShooterCharacter*,uint32> HitMap;
		TMap<AShooterCharacter*,uint32> HeadShotHitMap;
		for(FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start,HitTarget,FireHit);
			AShooterCharacter* Character = Cast<AShooterCharacter>(FireHit.GetActor());
			if(Character && HasAuthority() && InstigatorController)
			{
				const bool bHeadShot = FireHit.BoneName.ToString() == FString("head");
				if(bHeadShot)
				{
					if(HeadShotHitMap.Contains(Character)) HeadShotHitMap[Character]++;
					else HeadShotHitMap.Emplace(Character,1);
				}
				else
				{
					if(HitMap.Contains(Character)) HitMap[Character]++;
					else HitMap.Emplace(Character,1);
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
		}
		TArray<AShooterCharacter*> HitCharacters;

		TMap<AShooterCharacter*,float> DamageMap; // Maps Character hit to total damage
		for(auto HitPair : HitMap) // Calculate bodyshot damage by multiplying time hit * Damage - store in damage map
		{
			if(HitPair.Key)
			{
				DamageMap.Emplace(HitPair.Key,HitPair.Value * Damage);
				HitCharacters.AddUnique(HitPair.Key);
			}
		}
		for(auto HeadShotHitPair : HeadShotHitMap) // calculate head shot dmage by multi time hit * Headshotdamage 
		{
			if(HeadShotHitPair.Key)
			{
				if(DamageMap.Contains(HeadShotHitPair.Key)) DamageMap[HeadShotHitPair.Key] += HeadShotHitPair.Value * HeadShotDamage;
				else DamageMap.Emplace(HeadShotHitPair.Key, HeadShotHitPair.Value * HeadShotDamage);
				
				HitCharacters.AddUnique(HeadShotHitPair.Key);
			}
		}
		for(auto DamagePair : DamageMap)//Loop through damage map to get total damage for each character 
		{
			if(DamagePair.Key && InstigatorController)
			{
				bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
				if(HasAuthority() && bCauseAuthDamage)
				{
					UGameplayStatics::ApplyDamage(DamagePair.Key,DamagePair.Value,InstigatorController,this,UDamageType::StaticClass());
				}
			}
		}
		if(!HasAuthority() && bUseServerSideRewind)
		{
			ShooterOwnerCharacter = ShooterOwnerCharacter == nullptr ? Cast<AShooterCharacter>(OwnerPawn) : ShooterOwnerCharacter;
			ShooterOwnerPlayerController = ShooterOwnerPlayerController == nullptr ? Cast<AShooterPlayerController>(InstigatorController) : ShooterOwnerPlayerController;
			if(ShooterOwnerCharacter && ShooterOwnerPlayerController && ShooterOwnerCharacter->GetLagCompensation() )
			{
				ShooterOwnerCharacter->GetLagCompensation()->ShotgunServerScoreRequest(
					HitCharacters,
					Start,
					HitTargets,
					ShooterOwnerPlayerController->GetServerTime() - ShooterOwnerPlayerController->SingleTripTime);
			}
		}
	}
}
void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(MuzzleFlashSocket == nullptr) return;

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	
	for(uint32 i = 0; i < NumberOfPellets; i++)
	{
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f,SphereRadius);
		const FVector EndLoc = SphereCenter + RandVec;
		FVector ToEndLoc = EndLoc - TraceStart;
		ToEndLoc = TraceStart + ToEndLoc * TRACE / ToEndLoc.Size();
		
		HitTargets.Add(ToEndLoc);
	}
}
