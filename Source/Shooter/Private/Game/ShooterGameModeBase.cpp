// Hello :) 


#include "Game/ShooterGameModeBase.h"
#include "Character/ShooterCharacter.h"
#include "GameFramework/PlayerStart.h"
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
	AShooterPlayerState* AttackerPlayerState = AttackerController ? Cast<AShooterPlayerState>(AttackerController->PlayerState) : nullptr;
	AShooterPlayerState* VictimPlayerState = VictimController ? Cast<AShooterPlayerState>(VictimController->PlayerState) : nullptr;
	if(AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}
	if(VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	if(EliminatedCharacter)
	{
		EliminatedCharacter->Elim();
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
