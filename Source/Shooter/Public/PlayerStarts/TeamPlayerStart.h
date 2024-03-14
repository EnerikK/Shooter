// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "Shooter/Types/Team.h"
#include "TeamPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API ATeamPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere)
	ETeam Team;
	
};
