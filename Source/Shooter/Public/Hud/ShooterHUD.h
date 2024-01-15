// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ShooterHUD.generated.h"

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
	
	FORCEINLINE void SetHudPackage(const FHUDPackage& Package) {HudPackage = Package;}

private:
	
	FHUDPackage HudPackage;
	void DrawCrosshair(UTexture2D* Texture , FVector2d ViewportCenter,FVector2d Spread);

	UPROPERTY(EditAnywhere)
	float MaxCrosshairSpread = 15.f;

};
