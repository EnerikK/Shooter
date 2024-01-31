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

	virtual auto GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const -> void override;
	void UpdateTopScore(AShooterPlayerState* ScoringPlayer);
	
	UPROPERTY(Replicated)
	TArray<AShooterPlayerState*> TopScoringPlayer;

private:

	float TopScore = 0.f;
	
};
