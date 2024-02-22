// Hello :) 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

class AWeapon;
class AShooterPlayerController;
class AShooterCharacter;

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;

	UPROPERTY()
	AShooterCharacter* ShooterCharacter;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;
	
	UPROPERTY()
	bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotGunServerSideRewind
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<AShooterCharacter*,uint32> HeadShots;

	UPROPERTY()
	TMap<AShooterCharacter*,uint32> BodyShots;

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOOTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	friend AShooterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ShowFramePackage(const FFramePackage& Package,const FColor& Color);
	FServerSideRewindResult ServerSideRewind(AShooterCharacter* HitCharacter,const FVector_NetQuantize& TraceStart,const FVector_NetQuantize& HitLocation,float HitTime);
	FShotGunServerSideRewind ShotGunServerSideRewind(
		const TArray<AShooterCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime);
	
	UFUNCTION(Server,Reliable)
	void ServerScoreRequest(AShooterCharacter* HitCharacter ,const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,float HitTime,AWeapon* DamageCauser);

	UFUNCTION(Server,Reliable)
	void ShotgunServerScoreRequest(
		const TArray<AShooterCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime);
	
protected:
	virtual void BeginPlay() override;
	void SaveFramePackage(FFramePackage& Package);
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame,const FFramePackage& YoungerFrame ,float HitTime);
	FServerSideRewindResult ConfirmHit(
		const FFramePackage& Package,
		AShooterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation);

	void CacheBoxPosition(AShooterCharacter* HitCharacter,FFramePackage& OutFramePackage);
	void MoveBoxes(AShooterCharacter* HitCharacter,const FFramePackage& Package);
	void ResetHitBoxes(AShooterCharacter* HitCharacter,const FFramePackage& Package);
	void SaveFramePackage();
	void EnableCharacterMeshCollision(AShooterCharacter* HitCharacter,ECollisionEnabled::Type CollisionEnabled);
	FFramePackage GetFrameToCheck(AShooterCharacter* HitCharacter,float HitTime);
	
	/*Shotgun*/
	
	FShotGunServerSideRewind ShotGunConfirmHit(
		const TArray<FFramePackage>& FramePackages,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations);
	

private:

	UPROPERTY()
	AShooterCharacter* Character;

	UPROPERTY()
	AShooterPlayerController* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;
	
};
