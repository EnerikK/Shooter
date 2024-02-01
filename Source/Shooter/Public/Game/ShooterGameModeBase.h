// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ShooterGameModeBase.generated.h"

class AShooterPlayerController;
class AShooterCharacter;

namespace MatchState
{
	extern SHOOTER_API const FName Cooldown; //Match Duration has been reached display winner and begin cooldown  timer
	
	
}
/**
 * 
 */
UCLASS()
class SHOOTER_API AShooterGameModeBase : public AGameMode
{
	GENERATED_BODY()

public:

	AShooterGameModeBase();
	virtual void Tick(float DeltaSeconds) override;
	virtual void PlayerElimination(AShooterCharacter* EliminatedCharacter,AShooterPlayerController* VictimController,AShooterPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* EliminatedCharacter,AController* EliminatedController);

	UPROPERTY(EditDefaultsOnly)
	float InterventionTime = 3.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;
	
	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;
	
	float LevelStartingTime = 0.f;

	FORCEINLINE float GetCountdownTime() const {return CountdownTime;}

protected:
	
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
	
private:

	float CountdownTime = 0.f;
};
