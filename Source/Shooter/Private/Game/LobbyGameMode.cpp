// Hello :) 


#include "Game/LobbyGameMode.h"

#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 PlayerNumbers = GameState.Get()->PlayerArray.Num();

	UGameInstance* GameInstance = GetGameInstance();
	if(GameInstance)
	{
		UMultiplayerSessionsSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		check(Subsystem);
		if(PlayerNumbers == Subsystem->DesireNumPublicConnections)
		{
			UWorld* World  = GetWorld();
			if(World)
			{
				bUseSeamlessTravel = true;
				FString MatchType = Subsystem->DesiredMatchType;
				if(MatchType == "FreeForAll")
				{
					World->ServerTravel(FString("Game/Blueprints/Maps/ShooterMap?listen"));
				}
				else if (MatchType == "Teams")
				{
					World->ServerTravel(FString("Game/Blueprints/Maps/Team?listen"));
				}
				else if (MatchType == "CaptureTheFlag")
				{
					World->ServerTravel(FString("Game/Blueprints/Maps/CaptureTheFlag?listen"));
				}
			}
		}
	}
}
