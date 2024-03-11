// Hello :) 


#include "Weapon/ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapon/Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	UWorld*World = GetWorld();
	if(MuzzleFlashSocket && World)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		//From Muzzle to hit location from the trace under the crosshair
		FVector ToTarget =  HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();
		
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = GetOwner();
		SpawnParameters.Instigator = InstigatorPawn;

		AProjectile* SpawnProjectile = nullptr;
		if(bUseServerSideRewind)
		{
			if(InstigatorPawn->HasAuthority()) // Server
			{
				if(InstigatorPawn->IsLocallyControlled()) // Server,host - use replicated projectile
				{
					SpawnProjectile = World->SpawnActor<AProjectile>(Projectile,SocketTransform.GetLocation(),TargetRotation,SpawnParameters);
					SpawnProjectile->bUseServerSideRewind = false;
					SpawnProjectile->Damage = Damage;
					SpawnProjectile->HeadShotDamage = HeadShotDamage;
				}
				else // Server Not locally controlled - spawn non replicated projectile no ssr
				{
					SpawnProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass,SocketTransform.GetLocation(),TargetRotation,SpawnParameters);
					SpawnProjectile->bUseServerSideRewind = true;
				}
			}
			else // Client , using SSR 
			{
				if(InstigatorPawn->IsLocallyControlled()) // client locally controlled - spawn non-replicated projectile use ssr
				{
					SpawnProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass,SocketTransform.GetLocation(),TargetRotation,SpawnParameters);
					SpawnProjectile->bUseServerSideRewind = true;
					SpawnProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnProjectile->InitialVelocity = SpawnProjectile->GetActorForwardVector() * SpawnProjectile->InitialSpeed;
				}
				else // Client , not locally controlled - spawn no replicated projectile no ssr 
				{
					SpawnProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass,SocketTransform.GetLocation(),TargetRotation,SpawnParameters);
					SpawnProjectile->bUseServerSideRewind = false;
				}
			}
		}
		else // Weapon No SSR
		{
			if (InstigatorPawn->HasAuthority())
			{
				SpawnProjectile = World->SpawnActor<AProjectile>(Projectile,SocketTransform.GetLocation(),TargetRotation,SpawnParameters);
				SpawnProjectile->bUseServerSideRewind = false;
				SpawnProjectile->Damage = Damage;
				SpawnProjectile->HeadShotDamage = HeadShotDamage;
			}	
		}
	}
}
