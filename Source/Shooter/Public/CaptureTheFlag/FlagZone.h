// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Shooter/Types/Team.h"
#include "FlagZone.generated.h"

class USphereComponent;

UCLASS()
class SHOOTER_API AFlagZone : public AActor
{
	GENERATED_BODY()
	
public:
	
	AFlagZone();

	UPROPERTY(EditAnywhere)
	ETeam Team;


protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,
	bool bFromSweep,const FHitResult& SweepResult);

private:

	UPROPERTY(EditAnywhere)
	USphereComponent* ZoneSphere;

};
