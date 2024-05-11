// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "ShooterCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AShooterCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:

	AShooterCameraManager();

	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;
	
	UPROPERTY()
	float CrouchBlendDuration = 0.5f;

	float CrouchBlendTime;
	
};
