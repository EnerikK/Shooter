// Hello :) 


#include "PickUp/PickSpawnPoint.h"
#include "PickUp/PickUp.h"

APickSpawnPoint::APickSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

}
void APickSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	StartSpawnPickUpTimer((AActor*)nullptr);
	
}
void APickSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
void APickSpawnPoint::SpawnPickUp()
{
	int32 NumPickUpClasses = PickUpClasses.Num();
	if(NumPickUpClasses > 0)
	{
		int32 Selection = FMath::RandRange(0,NumPickUpClasses - 1);
		SpawnedPickUp = GetWorld()->SpawnActor<APickUps>(PickUpClasses[Selection],GetActorTransform());
		if(HasAuthority() && SpawnedPickUp)
		{
			SpawnedPickUp->OnDestroyed.AddDynamic(this,&APickSpawnPoint::StartSpawnPickUpTimer);
		}
	}
}
void APickSpawnPoint::SpawnPickUpTimerFinished()
{
	if(HasAuthority())
	{
		SpawnPickUp();
	}
}
void APickSpawnPoint::StartSpawnPickUpTimer(AActor* DestroyedActor)
{
	const float SpawnTime = FMath::FRandRange(SpawnPickUpTimeMin,SpawnPickUpTimeMax);
	GetWorldTimerManager().SetTimer(SpawnPickUpTimer,this,&APickSpawnPoint::SpawnPickUpTimerFinished,SpawnTime);

}

