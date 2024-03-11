// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "Game/ShooterGameModeBase.h"
#include "TeamsGameMode.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API ATeamsGameMode : public AShooterGameModeBase
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

protected:
	virtual void HandleMatchHasStarted() override;
	
};
