// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ShooterGameState.generated.h"


class AShooterPlayerState;
/**
 * 
 */
UCLASS()
class SHOOTER_API AShooterGameState : public AGameState
{
	GENERATED_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(AShooterPlayerState* ScoringPlayer);
	
	UPROPERTY(Replicated)
	TArray<AShooterPlayerState*> TopScoringPlayer;
	
	/*Teams*/
	
	void RedTeamScores();
	void BlueTeamScores();
	
	TArray<AShooterPlayerState*> RedTeam;
	TArray<AShooterPlayerState*> BlueTeam;

	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	float RedTeamScore = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;

	UFUNCTION()
	void OnRep_RedTeamScore();

	UFUNCTION()
	void OnRep_BlueTeamScore();
	
private:

	float TopScore = 0.f;
	
};
