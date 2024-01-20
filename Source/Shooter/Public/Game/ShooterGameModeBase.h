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

	virtual void PlayerElimination(AShooterCharacter* EliminatedCharacter,AShooterPlayerController* VictimController,AShooterPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* EliminatedCharacter,AController* EliminatedController);
};
