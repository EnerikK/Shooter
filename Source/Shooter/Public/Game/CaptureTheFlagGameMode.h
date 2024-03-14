// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "Game/TeamsGameMode.h"
#include "CaptureTheFlagGameMode.generated.h"

class AFlagZone;
class AFlag;
class AShooterCharacter;
class AShooterPlayerController;
/**
 * 
 */
UCLASS()
class SHOOTER_API ACaptureTheFlagGameMode : public ATeamsGameMode
{
	GENERATED_BODY()

public:

	virtual void PlayerElimination(AShooterCharacter* EliminatedCharacter, AShooterPlayerController* VictimController, AShooterPlayerController* AttackerController) override;
	void FlagCaptured(AFlag* Flag , AFlagZone* Zone);
};
