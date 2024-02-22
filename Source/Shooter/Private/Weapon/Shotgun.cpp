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
		for(FVector_NetQuantize HitTarget : HitTargets)
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
		
		for(auto HitPair : HitMap)
		{
			if(InstigatorController)
			{
				if(HitPair.Key && HasAuthority() && InstigatorController)
				{
					if(HasAuthority() && !bUseServerSideRewind)
					{
						UGameplayStatics::ApplyDamage(HitPair.Key,Damage * HitPair.Value,InstigatorController,this,UDamageType::StaticClass());
					}
				}
				HitCharacters.Add(HitPair.Key);
			}
		}
		if(!HasAuthority() && bUseServerSideRewind)
		{
			ShooterOwnerCharacter = ShooterOwnerCharacter == nullptr ? Cast<AShooterCharacter>(OwnerPawn) : ShooterOwnerCharacter;
			ShooterOwnerPlayerController = ShooterOwnerPlayerController == nullptr ? Cast<AShooterPlayerController>(InstigatorController) : ShooterOwnerPlayerController;
			if(ShooterOwnerCharacter && ShooterOwnerPlayerController && ShooterOwnerCharacter->GetLagCompensation() && ShooterOwnerCharacter->IsLocallyControlled())
			{
				ShooterOwnerCharacter->GetLagCompensation()->ShotgunServerScoreRequest(
					HitCharacters,
					Start,HitTargets,
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
