// Hello :) 


#include "Hud/KillAnnouncement.h"

#include "Components/TextBlock.h"

void UKillAnnouncement::SetKillAnnouncementText(FString AttackerName, FString VictimName)
{
	FString KillAnnouncementText = FString::Printf(TEXT("%s Killed %s"),*AttackerName,*VictimName);
	if(AnnouncementText)
	{
		AnnouncementText->SetText(FText::FromString(KillAnnouncementText));
	}
}
