// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "Game/ShooterGameModeBase.h"
#include "TeamsGameMode.generated.h"

class AShooterCharacter;
class AShooterPlayerController;

/**
 * 
 */
UCLASS()
class SHOOTER_API ATeamsGameMode : public AShooterGameModeBase
{
	GENERATED_BODY()

public:

	ATeamsGameMode();
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;
	virtual void PlayerElimination(AShooterCharacter* EliminatedCharacter, AShooterPlayerController* VictimController, AShooterPlayerController* AttackerController) override;
protected:
	virtual void HandleMatchHasStarted() override;
	
};
