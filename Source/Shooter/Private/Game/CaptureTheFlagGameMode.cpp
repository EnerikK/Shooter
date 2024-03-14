// Hello :) 


#include "Game/CaptureTheFlagGameMode.h"

#include "CaptureTheFlag/FlagZone.h"
#include "GameState/ShooterGameState.h"
#include "Weapon/Flag.h"

void ACaptureTheFlagGameMode::PlayerElimination(AShooterCharacter* EliminatedCharacter,
                                                AShooterPlayerController* VictimController, AShooterPlayerController* AttackerController)
{
	AShooterGameModeBase::PlayerElimination(EliminatedCharacter,VictimController,AttackerController);
	
}

void ACaptureTheFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* Zone)
{
	bool bValidCapture = Flag->GetTeam() != Zone->Team;
	AShooterGameState* ShooterGameState = Cast<AShooterGameState>(GameState);
	if(ShooterGameState && bValidCapture)
	{
		if(Zone->Team == ETeam::ET_BlueTeam)
		{
			ShooterGameState->BlueTeamScores();
		}
		if(Zone->Team == ETeam::ET_RedTeam)
		{
			ShooterGameState->RedTeamScores();
		}
	}
	
}
