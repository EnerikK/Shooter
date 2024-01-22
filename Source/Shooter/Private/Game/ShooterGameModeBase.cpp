// Hello :) 


#include "Game/ShooterGameModeBase.h"
#include "Character/ShooterCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ShooterPlayerController.h"
#include "PlayerState/ShooterPlayerState.h"

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
