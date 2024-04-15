// Hello :) 


#include "Hud/ShooterHUD.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "GameFramework/PlayerController.h"
#include "Hud/Announcement.h"
#include "Hud/HudOverlay.h"
#include "Hud/KillAnnouncement.h"

void AShooterHUD::BeginPlay()
{
	Super::BeginPlay();
}

void AShooterHUD::AddHudOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if(PlayerController && HudOverlayClass)
	{
		HudOverlay = CreateWidget<UHudOverlay>(PlayerController,HudOverlayClass);
		HudOverlay->AddToViewport();
	}
}
void AShooterHUD::AddAnnouncement()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if(PlayerController && AnnouncementClass)
	{
		Announcement = CreateWidget<UAnnouncement>(PlayerController,AnnouncementClass);
		Announcement->AddToViewport();
	}
	
}
void AShooterHUD::AddKillAnnouncement(FString Attacker,FString Victim)
{
	OwningPlayer = OwningPlayer == nullptr ? GetOwningPlayerController() : OwningPlayer;
	if(OwningPlayer && KillAnnouncementClass)
	{
		UKillAnnouncement* KillAnnouncementWidget = CreateWidget<UKillAnnouncement>(OwningPlayer,KillAnnouncementClass);
		if(KillAnnouncementWidget)
		{
			KillAnnouncementWidget->SetKillAnnouncementText(Attacker,Victim);
			KillAnnouncementWidget->AddToViewport();
			for(auto Message : KillMessages)
			{
				if(Message && Message->AnnouncementBox)
				{
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Message->AnnouncementBox);
					if(CanvasSlot)
					{
						FVector2D Position = CanvasSlot->GetPosition();
						FVector2D NewPosition(CanvasSlot->GetPosition().X,Position.Y - CanvasSlot->GetSize().Y);
						CanvasSlot->SetPosition(NewPosition);
					}
				}
			}
			KillMessages.Add(KillAnnouncementWidget);
			
			FTimerHandle KillMessageTimer;
			FTimerDelegate KillMessageDelegate;
			KillMessageDelegate.BindUFunction(this,FName("KillAnnouncementTimerFinished"),KillAnnouncementWidget);
			GetWorldTimerManager().SetTimer(KillMessageTimer,KillMessageDelegate,KillAnnouncementTime,false); 
		}
	}
}
void AShooterHUD::KillAnnouncementTimerFinished(UKillAnnouncement* MessageToRemove)
{
	if(MessageToRemove)
	{
		MessageToRemove->RemoveFromParent();
	}
}
void AShooterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2d ViewportSize;
	if(GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
		
		float SpreadScale = MaxCrosshairSpread* HudPackage.CrosshairSpread;
		
		if (HudPackage.CrosshairCenter)
		{
			FVector2d Spread(0.f,0.f);
			DrawCrosshair(HudPackage.CrosshairCenter, ViewportCenter,Spread,HudPackage.CrosshairColor);
		}
		if (HudPackage.CrosshairLeft)
		{
			FVector2d Spread(-SpreadScale,0.f);
			DrawCrosshair(HudPackage.CrosshairLeft, ViewportCenter,Spread,HudPackage.CrosshairColor);
		}
		if (HudPackage.CrosshairRight)
		{
			FVector2d Spread(SpreadScale,0.f);
			DrawCrosshair(HudPackage.CrosshairRight, ViewportCenter,Spread,HudPackage.CrosshairColor);
		}
		if (HudPackage.CrosshairTop)
		{
			FVector2d Spread(0.f,-SpreadScale);
			DrawCrosshair(HudPackage.CrosshairTop, ViewportCenter,Spread,HudPackage.CrosshairColor);
		}
		if (HudPackage.CrosshairBottom)
		{
			FVector2d Spread(0.f,SpreadScale);
			DrawCrosshair(HudPackage.CrosshairBottom, ViewportCenter,Spread,HudPackage.CrosshairColor);
		}
	}
}
void AShooterHUD::DrawCrosshair(UTexture2D* Texture, FVector2d ViewportCenter,FVector2d Spread,FLinearColor CrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y);
	
	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		CrosshairColor
	);
}

