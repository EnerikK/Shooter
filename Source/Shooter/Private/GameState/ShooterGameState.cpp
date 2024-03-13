// Hello :) 


#include "GameState/ShooterGameState.h"
#include "Net/UnrealNetwork.h"
#include "Player/ShooterPlayerController.h"
#include "PlayerState/ShooterPlayerState.h"


void AShooterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AShooterGameState,TopScoringPlayer);
	DOREPLIFETIME(AShooterGameState,RedTeamScore);
	DOREPLIFETIME(AShooterGameState,BlueTeamScore);
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

void AShooterGameState::RedTeamScores()
{
	++RedTeamScore;
	AShooterPlayerController* ShooterPlayer = Cast<AShooterPlayerController>(GetWorld()->GetFirstPlayerController());
	if(ShooterPlayer)
	{
		ShooterPlayer->SetHudRedTeamScore(RedTeamScore);
	}
}
void AShooterGameState::BlueTeamScores()
{
	++BlueTeamScore;
	AShooterPlayerController* ShooterPlayer = Cast<AShooterPlayerController>(GetWorld()->GetFirstPlayerController());
	if(ShooterPlayer)
	{
		ShooterPlayer->SetHudBlueTeamScore(BlueTeamScore);
	}
}
void AShooterGameState::OnRep_RedTeamScore()
{
	AShooterPlayerController* ShooterPlayer = Cast<AShooterPlayerController>(GetWorld()->GetFirstPlayerController());
	if(ShooterPlayer)
	{
		ShooterPlayer->SetHudRedTeamScore(RedTeamScore);
	}
}
void AShooterGameState::OnRep_BlueTeamScore()
{
	AShooterPlayerController* ShooterPlayer = Cast<AShooterPlayerController>(GetWorld()->GetFirstPlayerController());
	if(ShooterPlayer)
	{
		ShooterPlayer->SetHudBlueTeamScore(BlueTeamScore);
	}
}
