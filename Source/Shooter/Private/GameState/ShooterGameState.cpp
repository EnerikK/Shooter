// Hello :) 


#include "GameState/ShooterGameState.h"
#include "Net/UnrealNetwork.h"
#include "PlayerState/ShooterPlayerState.h"


void AShooterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShooterGameState,TopScoringPlayer);
}

void AShooterGameState::UpdateTopScore(AShooterPlayerState* ScoringPlayer)
{
	if(TopScoringPlayer.Num() == 0)
	{
		TopScoringPlayer.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayer.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayer.Empty();
		TopScoringPlayer.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}
