// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ShooterGameModeBase.generated.h"

class AShooterPlayerController;
class AShooterCharacter;
/**
 * 
 */
UCLASS()
class SHOOTER_API AShooterGameModeBase : public AGameMode
{
	GENERATED_BODY()

public:

	AShooterGameModeBase();
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditDefaultsOnly)
	float InterventionTime = 3.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 20.f;

	float LevelStartingTime = 0.f;

	
	virtual void PlayerElimination(AShooterCharacter* EliminatedCharacter,AShooterPlayerController* VictimController,AShooterPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* EliminatedCharacter,AController* EliminatedController);

protected:
	
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
	
private:

	float CountdownTime = 0.f;
};
