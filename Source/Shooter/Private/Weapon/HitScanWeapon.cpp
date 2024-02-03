// Hello :) 


#include "Weapon/HitScanWeapon.h"
#include "Character/ShooterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "WeaponTypes.h"
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
		AShooterCharacter* Character = Cast<AShooterCharacter>(FireHit.GetActor());
		if(Character && HasAuthority() && InstigatorController)
		{
			UGameplayStatics::ApplyDamage(Character,Damage,InstigatorController,this,UDamageType::StaticClass());
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
		FVector End = bUseScatter ? TraceEndWithScatter(TraceStart,HitTarget) : TraceStart + (HitTarget - TraceStart)* 1.25f;
		World->LineTraceSingleByChannel(OutHit,TraceStart,End,ECC_Visibility);
		FVector BeamEnd = End;
		if(OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
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
FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f,SphereRadius);
	FVector EndLoc = SphereCenter + RandVec;
	FVector ToEndLoc = EndLoc - TraceStart;
		
	/*DrawDebugSphere(GetWorld(),SphereCenter,SphereRadius,12,FColor::Red,true);
	DrawDebugSphere(GetWorld(),EndLoc,4.f,12,FColor::Orange,true);
	DrawDebugLine(GetWorld(),TraceStart,FVector(TraceStart + ToEndLoc * TRACE / ToEndLoc.Size()),FColor::Black,true);*/

	return  FVector(TraceStart + ToEndLoc * TRACE / ToEndLoc.Size());
}

