// Hello :) 


#include "Game/LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 PlayerNumbers = GameState.Get()->PlayerArray.Num();
	if(PlayerNumbers == 2)
	{
		UWorld* World  = GetWorld();
		if(World)
		{
			bUseSeamlessTravel = true; 
			World->ServerTravel(FString("Game/Blueprints/Maps/ShooterMap?listen"));
		}
	}
	
}
