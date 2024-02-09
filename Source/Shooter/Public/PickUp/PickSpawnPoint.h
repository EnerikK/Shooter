// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickSpawnPoint.generated.h"

class APickUps;
UCLASS()
class SHOOTER_API APickSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	APickSpawnPoint();
	virtual void Tick(float DeltaTime) override;


protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<APickUps>> PickUpClasses;

	UPROPERTY()
	APickUps* SpawnedPickUp;

	void SpawnPickUp();
	void SpawnPickUpTimerFinished();
	UFUNCTION()
	void StartSpawnPickUpTimer(AActor* DestroyedActor);

private:
	
	FTimerHandle SpawnPickUpTimer;

	UPROPERTY(EditAnywhere)
	float SpawnPickUpTimeMin;
	
	UPROPERTY(EditAnywhere)
	float SpawnPickUpTimeMax;
	
};
