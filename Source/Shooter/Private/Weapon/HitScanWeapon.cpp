// Hello :) 


#include "Weapon/HitScanWeapon.h"
#include "Character/ShooterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Player/ShooterPlayerController.h"
#include "Sound/SoundCue.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(Start,HitTarget,FireHit);
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(FireHit.GetActor());
		if(ShooterCharacter && InstigatorController)
		{
			bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
			if(HasAuthority() && bCauseAuthDamage)
			{
				const float DamageToCause = FireHit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;
				UGameplayStatics::ApplyDamage(ShooterCharacter,DamageToCause,InstigatorController,this,UDamageType::StaticClass());
			}
			if(!HasAuthority()  && bUseServerSideRewind)
			{
				ShooterOwnerCharacter = ShooterOwnerCharacter == nullptr ? Cast<AShooterCharacter>(OwnerPawn) : ShooterOwnerCharacter;
				ShooterOwnerPlayerController = ShooterOwnerPlayerController == nullptr ? Cast<AShooterPlayerController>(InstigatorController) : ShooterOwnerPlayerController;
				if(ShooterOwnerCharacter && ShooterOwnerPlayerController && ShooterOwnerCharacter->GetLagCompensation() && ShooterOwnerCharacter->IsLocallyControlled())
				{
					ShooterOwnerCharacter->GetLagCompensation()->ServerScoreRequest(
						ShooterCharacter,
						Start,
						HitTarget,
						ShooterOwnerPlayerController->GetServerTime() - ShooterOwnerPlayerController->SingleTripTime,
						this);	
				}
			}
			
		}
		if(ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),ImpactParticles,FireHit.ImpactPoint,FireHit.ImpactNormal.Rotation());
		}
		if(HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this,HitSound,FireHit.ImpactPoint);
		}
		if(MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),MuzzleFlash,SocketTransform);
		}
		if(FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this,FireSound,GetActorLocation());
		}
	}
}
void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart,const FVector& HitTarget,FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if(World)
	{
		FVector End = TraceStart + (HitTarget - TraceStart)* 1.25f;
		World->LineTraceSingleByChannel(OutHit,TraceStart,End,ECC_Visibility);
		FVector BeamEnd = End;
		if(OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		else
		{
			OutHit.ImpactPoint = End;
		}
		/*DrawDebugSphere(GetWorld(),BeamEnd,16.f,12,FColor::Orange,true);*/
		if(BeamParticle)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(World,BeamParticle,TraceStart,FRotator::ZeroRotator);
			if(Beam)
			{
				Beam->SetVectorParameter(FName("Target"),BeamEnd);
			}
		}
	}
}

