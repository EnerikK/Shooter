// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Shooter/Types/Team.h"
#include "ShooterPlayerState.generated.h"

class AShooterPlayerController;
class AShooterCharacter;
/**
 * 
 */
UCLASS()
class SHOOTER_API AShooterPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Score() override;
	UFUNCTION()
	virtual void OnRep_Defeats();
	void AddToScore(float ScoreAmount);
	void AddToDefeats(int32 DefeatsAmount);

	FORCEINLINE ETeam GetTeam() const {return Team;}
	FORCEINLINE void SetTeam(ETeam TeamToSet);

private:

	UPROPERTY()
	AShooterCharacter* Character;
	UPROPERTY()
	AShooterPlayerController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats) 
	int32 Defeats;

	UPROPERTY(ReplicatedUsing = OnRep_Team)
	ETeam Team = ETeam::ET_NoTeam;

	UFUNCTION()
	void OnRep_Team();

	
};

