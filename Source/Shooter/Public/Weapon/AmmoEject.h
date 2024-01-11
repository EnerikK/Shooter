// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AmmoEject.generated.h"

class USoundCue;

UCLASS()
class SHOOTER_API AAmmoEject : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AAmmoEject();
	
protected:
	
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	float ShellEjectionImpulse;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp,AActor* OtherActor,UPrimitiveComponent* OtherComp,FVector NormalImpulse,const FHitResult& Hit);

private:

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Ammo;

	UPROPERTY(EditAnywhere)
	USoundCue* ShellSound;
	
};
