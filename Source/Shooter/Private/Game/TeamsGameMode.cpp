// Hello :) 


#include "Game/TeamsGameMode.h"

#include "GameState/ShooterGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ShooterPlayerController.h"
#include "PlayerState/ShooterPlayerState.h"


ATeamsGameMode::ATeamsGameMode()
{
	bTeamsMatch = true;
}
void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	AShooterGameState* ShooterGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
	if(ShooterGameState)
	{
		AShooterPlayerState* ShooterPlayerState = NewPlayer->GetPlayerState<AShooterPlayerState>();
		if(ShooterPlayerState && ShooterPlayerState->GetTeam() == ETeam::ET_NoTeam)
		{
			if(ShooterGameState->BlueTeam.Num() >= ShooterGameState->RedTeam.Num())
			{
				ShooterGameState->RedTeam.AddUnique(ShooterPlayerState);
				ShooterPlayerState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				ShooterGameState->BlueTeam.AddUnique(ShooterPlayerState);
				ShooterPlayerState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}
void ATeamsGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	AShooterGameState* ShooterGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
	AShooterPlayerState* ShooterPlayerState = Exiting->GetPlayerState<AShooterPlayerState>();

	if(ShooterGameState && ShooterPlayerState)
	{
		if(ShooterGameState->RedTeam.Contains(ShooterPlayerState))
		{
			ShooterGameState->RedTeam.Remove(ShooterPlayerState);
		}
		if(ShooterGameState->BlueTeam.Contains(ShooterPlayerState))
		{
			ShooterGameState->BlueTeam.Remove(ShooterPlayerState);
		}
	}
	
}

float ATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	AShooterPlayerState* AttackerPlayerState = Attacker->GetPlayerState<AShooterPlayerState>();
	AShooterPlayerState* VictimPlayerState = Victim->GetPlayerState<AShooterPlayerState>();
	if(AttackerPlayerState == nullptr || VictimPlayerState == nullptr) return BaseDamage;
	if(VictimPlayerState == AttackerPlayerState)
	{
		return BaseDamage;
	}
	if(AttackerPlayerState->GetTeam() == VictimPlayerState->GetTeam())
	{
		return 0.f;
	}
	return BaseDamage;
}

void ATeamsGameMode::PlayerElimination(AShooterCharacter* EliminatedCharacter,
	AShooterPlayerController* VictimController, AShooterPlayerController* AttackerController)
{
	Super::PlayerElimination(EliminatedCharacter, VictimController, AttackerController);
	
	AShooterGameState* ShooterGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
	AShooterPlayerState* AttackerPlayerState = AttackerController ? Cast<AShooterPlayerState>(AttackerController->PlayerState) : nullptr;
	if(ShooterGameState && AttackerPlayerState)
	{
		if(AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			ShooterGameState->BlueTeamScore;
		}
		if(AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			ShooterGameState->RedTeamScore;
		}
	}
	
}
void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	AShooterGameState* ShooterGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
	if(ShooterGameState)
	{
		for(auto PlayerState : ShooterGameState->PlayerArray)
		{
			AShooterPlayerState* ShooterPlayerState = Cast<AShooterPlayerState>(PlayerState.Get());
			if(ShooterPlayerState && ShooterPlayerState->GetTeam() == ETeam::ET_NoTeam)
			{
				if(ShooterGameState->BlueTeam.Num() >= ShooterGameState->RedTeam.Num())
				{
					ShooterGameState->RedTeam.AddUnique(ShooterPlayerState);
					ShooterPlayerState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					ShooterGameState->BlueTeam.AddUnique(ShooterPlayerState);
					ShooterPlayerState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}
