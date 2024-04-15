// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "GameFramework/Actor.h"
#include "PickUp.generated.h"

class USphereComponent;
class USoundCue;

UCLASS()
class SHOOTER_API APickUps : public AActor
{
	GENERATED_BODY()
	
public:
	
	APickUps();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;


protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,
		bool bFromSweep,const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 45.f;

private:

	UPROPERTY(EditAnywhere)
	USphereComponent* OverlapSphere;
	
	UPROPERTY(EditAnywhere)
	USoundCue* PickUpSound;
	
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PickUpMesh;

	UPROPERTY(VisibleAnywhere)
	UNiagaraComponent* PickupEffectComponent;

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* PickUpEffect;

	FTimerHandle BindOverlapTimer;

	float BindOverlapTime = 0.25f;

	void BindOverlapTimerFinished();
	
};
