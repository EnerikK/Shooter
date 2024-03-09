// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KillAnnouncement.generated.h"

class UTextBlock;
class UHorizontalBox;
/**
 * 
 */
UCLASS()
class SHOOTER_API UKillAnnouncement : public UUserWidget
{
	GENERATED_BODY()

public:

	void SetKillAnnouncementText(FString AttackerName,FString VictimName);

	UPROPERTY(meta=(BindWidget))
	UHorizontalBox* AnnouncementBox;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* AnnouncementText;
	
};
