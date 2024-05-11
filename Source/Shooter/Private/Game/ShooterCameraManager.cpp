// Hello :) 


#include "Game/ShooterCameraManager.h"

#include "Character/ShooterCharacter.h"
#include "Components/CapsuleComponent.h"

AShooterCameraManager::AShooterCameraManager()
{
}

void AShooterCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	Super::UpdateViewTarget(OutVT, DeltaTime);

	
	if (AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetOwningPlayerController()->GetPawn()))
	{
		UShooterMovementComponent* SMC = ShooterCharacter->GetShooterCharacterComponent();
		FVector TargetCrouchOffset = FVector(
			0,
			0,
			SMC->GetCrouchedHalfHeight() - ShooterCharacter->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
		);
		FVector Offset = FMath::Lerp(FVector::ZeroVector, TargetCrouchOffset, FMath::Clamp(CrouchBlendTime / CrouchBlendDuration, 0.f, 1.f));

		if (SMC->IsCrouching())
		{
			CrouchBlendTime = FMath::Clamp(CrouchBlendTime + DeltaTime, 0.f, CrouchBlendDuration);
			Offset -= TargetCrouchOffset;
		}
		else
		{
			CrouchBlendTime = FMath::Clamp(CrouchBlendTime - DeltaTime, 0.f, CrouchBlendDuration);
		}

		OutVT.POV.Location += Offset;
	}
}
