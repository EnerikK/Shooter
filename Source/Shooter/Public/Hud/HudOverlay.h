// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HudOverlay.generated.h"

class UTextBlock;
class UProgressBar;
/**
 * 
 */
UCLASS()
class SHOOTER_API UHudOverlay : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAmount;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefeatsAmount;
	
	
};
