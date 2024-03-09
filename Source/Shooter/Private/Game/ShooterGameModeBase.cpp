// Hello :) 


#include "Game/ShooterGameModeBase.h"
#include "Character/ShooterCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "GameState/ShooterGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ShooterPlayerController.h"
#include "PlayerState/ShooterPlayerState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}
AShooterGameModeBase::AShooterGameModeBase()
{
	bDelayedStart = true;
	
}
void AShooterGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AShooterGameModeBase::OnMatchStateSet()
{
	Super::OnMatchStateSet();
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		AShooterPlayerController* ShooterPlayer = Cast<AShooterPlayerController>(*Iterator);
		if(ShooterPlayer)
		{
			ShooterPlayer->OnMatchStateSet(MatchState);	
		}
	}
	
}
void AShooterGameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = InterventionTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if(CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = InterventionTime + MatchTime - GetWorld()->GetTimeSeconds()+LevelStartingTime;
		if(CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + InterventionTime + MatchTime - GetWorld()->GetTimeSeconds()+LevelStartingTime;
		if(CountdownTime < 0.f)
		{
			RestartGame();
		}
	}
}
void AShooterGameModeBase::PlayerElimination(AShooterCharacter* EliminatedCharacter,
                                             AShooterPlayerController* VictimController, AShooterPlayerController* AttackerController)
{
	if(AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if(VictimController == nullptr || VictimController->PlayerState == nullptr) return;
	AShooterPlayerState* AttackerPlayerState = AttackerController ? Cast<AShooterPlayerState>(AttackerController->PlayerState) : nullptr;
	AShooterPlayerState* VictimPlayerState = VictimController ? Cast<AShooterPlayerState>(VictimController->PlayerState) : nullptr;

	AShooterGameState* ShooterGameState = GetGameState<AShooterGameState>();
	
	if(AttackerPlayerState && AttackerPlayerState != VictimPlayerState && ShooterGameState)
	{
		TArray<AShooterPlayerState*> PlatersCurrentlyInTheLead;
		for(auto LeadPlayer : ShooterGameState->TopScoringPlayer)
		{
			PlatersCurrentlyInTheLead.Add(LeadPlayer);
		}
		AttackerPlayerState->AddToScore(1.f);
		ShooterGameState->UpdateTopScore(AttackerPlayerState);
		if(ShooterGameState->TopScoringPlayer.Contains(AttackerPlayerState))
		{
			AShooterCharacter* Leader = Cast<AShooterCharacter>(AttackerPlayerState->GetPawn());
			if(Leader)
			{
				Leader->MulticastGainedTheLead();
			}
		}
		for(int32 i = 0; i < PlatersCurrentlyInTheLead.Num(); i++)
		{
			if(!ShooterGameState->TopScoringPlayer.Contains(PlatersCurrentlyInTheLead[i]))
			{
				AShooterCharacter* Loser = Cast<AShooterCharacter>(PlatersCurrentlyInTheLead[i]->GetPawn());
				if(Loser)
				{
					Loser->MulticastLostTheLead();
				}
			}
		}
	}
	if(VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	if(EliminatedCharacter)
	{
		EliminatedCharacter->Elim(false);
	}
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)//Looping player controllers
	{
		AShooterPlayerController* ShooterPlayer = Cast<AShooterPlayerController>(*It);
		if(ShooterPlayer && AttackerPlayerState && VictimPlayerState)
		{
			ShooterPlayer->BroadcastKill(AttackerPlayerState,VictimPlayerState);
		}
	}
}
void AShooterGameModeBase::RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController)
{
	if(EliminatedCharacter)
	{
		EliminatedCharacter->Reset();
		EliminatedCharacter->Destroy();
	}
	if(EliminatedController)
	{
		TArray<AActor*> PlayerStart;
		UGameplayStatics::GetAllActorsOfClass(this,APlayerStart::StaticClass(),PlayerStart);
		int32 Selection = FMath::RandRange(0,PlayerStart.Num() - 1);
		RestartPlayerAtPlayerStart(EliminatedController,PlayerStart[Selection]);
	}
}
void AShooterGameModeBase::PlayerLeftGame(AShooterPlayerState* PlayerLeaving)
{
	if(PlayerLeaving == nullptr) return;
	AShooterGameState* ShooterGameState = GetGameState<AShooterGameState>();
	if(ShooterGameState && ShooterGameState->TopScoringPlayer.Contains(PlayerLeaving))
	{
		ShooterGameState->TopScoringPlayer.Remove(PlayerLeaving);
	}
	AShooterCharacter* CharacterLeaving = Cast<AShooterCharacter>(PlayerLeaving->GetPawn());
	if(CharacterLeaving)
	{
		CharacterLeaving->Elim(true);
	}
}
