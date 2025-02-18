// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ShooterHUD.generated.h"

class UKillAnnouncement;
class UAnnouncement;
class UHudOverlay;
class UTexture2D;

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
	
public:
	
	UTexture2D* CrosshairCenter;
	UTexture2D* CrosshairLeft;
	UTexture2D* CrosshairRight;
	UTexture2D* CrosshairTop;
	UTexture2D* CrosshairBottom;
	float CrosshairSpread;
	FLinearColor CrosshairColor;

};

/**
 * 
 */
UCLASS()
class SHOOTER_API AShooterHUD : public AHUD
{
	GENERATED_BODY()

public:

	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere,Category="Player Stats")
	TSubclassOf<UUserWidget> HudOverlayClass;
	UPROPERTY()
	UHudOverlay* HudOverlay;
	
	void AddHudOverlay();

	UPROPERTY(EditAnywhere,Category="Announcements")
	TSubclassOf<UUserWidget> AnnouncementClass;
	UPROPERTY()
	UAnnouncement* Announcement;
	void AddAnnouncement();
	void AddKillAnnouncement(FString Attacker,FString Victim);

	FORCEINLINE void SetHudPackage(const FHUDPackage& Package) {HudPackage = Package;}

protected:

	virtual void BeginPlay() override;

private:

	UPROPERTY()
	APlayerController* OwningPlayer;
	
	FHUDPackage HudPackage;
	void DrawCrosshair(UTexture2D* Texture , FVector2d ViewportCenter,FVector2d Spread,FLinearColor CrosshairColor);

	UPROPERTY(EditAnywhere)
	float MaxCrosshairSpread = 15.f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UKillAnnouncement> KillAnnouncementClass;

	UPROPERTY(EditAnywhere)
	float KillAnnouncementTime = 3.5f;

	UFUNCTION()
	void KillAnnouncementTimerFinished(UKillAnnouncement* MessageToRemove);

	UPROPERTY()
	TArray<UKillAnnouncement*> KillMessages;
};
