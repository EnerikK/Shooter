// Hello :) 


#include "Hud/ShooterHUD.h"

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
			DrawCrosshair(HudPackage.CrosshairCenter, ViewportCenter,Spread);
		}
		if (HudPackage.CrosshairLeft)
		{
			FVector2d Spread(-SpreadScale,0.f);
			DrawCrosshair(HudPackage.CrosshairLeft, ViewportCenter,Spread);
		}
		if (HudPackage.CrosshairRight)
		{
			FVector2d Spread(SpreadScale,0.f);
			DrawCrosshair(HudPackage.CrosshairRight, ViewportCenter,Spread);
		}
		if (HudPackage.CrosshairTop)
		{
			FVector2d Spread(0.f,-SpreadScale);
			DrawCrosshair(HudPackage.CrosshairTop, ViewportCenter,Spread);
		}
		if (HudPackage.CrosshairBottom)
		{
			FVector2d Spread(0.f,SpreadScale);
			DrawCrosshair(HudPackage.CrosshairBottom, ViewportCenter,Spread);
		}
	}
}

void AShooterHUD::DrawCrosshair(UTexture2D* Texture, FVector2d ViewportCenter,FVector2d Spread)
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
		FLinearColor::White
	);
}
