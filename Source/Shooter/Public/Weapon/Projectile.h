// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UProjectileMovementComponent;
class UBoxComponent;

UCLASS()
class SHOOTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	

	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;
	void StartDestroyTimer();
	void DestroyTimerFinished();
	void ExplodeDamage();
	
	/*Use with server side rewind*/
	UPROPERTY(EditAnywhere)
	bool bUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 InitialVelocity;

	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000.f;
	
	float Damage = 20.f;
	
protected:
	
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp,AActor* OtherActor,UPrimitiveComponent* OtherComp,FVector NormalImpulse,const FHitResult& Hit);
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;
	
	UPROPERTY(EditAnywhere)
	USoundCue* ImpactSound;

	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 200.f;
	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.f;
	
private:
	
	UPROPERTY(EditAnywhere)
	UParticleSystem* Tracer;

	UParticleSystemComponent* TraceComponent;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 2.f;
	
};
