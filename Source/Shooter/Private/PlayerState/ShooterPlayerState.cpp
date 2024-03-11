// Hello :) 


#include "PlayerState/ShooterPlayerState.h"
#include "Character/ShooterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Player/ShooterPlayerController.h"

void AShooterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShooterPlayerState,Defeats);
	DOREPLIFETIME(AShooterPlayerState,Team);
}
void AShooterPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	Character = Character == nullptr ? Cast<AShooterCharacter>(GetPawn()) : Character;
	if(Character && Character->Controller)
	{
		Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
		if(Controller)
		{
			Controller->SetHudScore(GetScore());
		}
	}
}
void AShooterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<AShooterCharacter>(GetPawn()) : Character;
	if(Character && Character->Controller)
	{
		Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
		if(Controller)
		{
			Controller->SetHudScore(GetScore());
		}
	}
}
void AShooterPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	Character = Character == nullptr ? Cast<AShooterCharacter>(GetPawn()) : Character;
	if(Character && Character->Controller)
	{
		Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
		if(Controller)
		{
			Controller->SetHudDefeats(Defeats);
		}
	}
}
void AShooterPlayerState::SetTeam(ETeam TeamToSet)
{
	Team = TeamToSet;
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetPawn());
	if(ShooterCharacter)
	{
		ShooterCharacter->SetTeamColor(Team);
	}
}
void AShooterPlayerState::OnRep_Team()
{
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetPawn());
	if(ShooterCharacter)
	{
		ShooterCharacter->SetTeamColor(Team);
	}
}
void AShooterPlayerState::OnRep_Defeats()
{

	Character = Character == nullptr ? Cast<AShooterCharacter>(GetPawn()) : Character;
	if(Character && Character->Controller)
	{
		Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
		if(Controller)
		{
			Controller->SetHudDefeats(Defeats);
		}
	}
	
}
