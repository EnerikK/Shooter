// Hello :) 


#include "Components/LagCompensationComponent.h"
#include "Character/ShooterCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Shooter/Shooter.h"
#include "Weapon/Weapon.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}
void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
	
}
void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SaveFramePackage();

}
void ULagCompensationComponent::SaveFramePackage()
{
	if(Character == nullptr || Character->HasAuthority()) return;
	if(FrameHistory.Num() <= 1)
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
	else
	{
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		while (HistoryLength > MaxRecordTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);

		/*ShowFramePackage(ThisFrame,FColor::Red);*/
	}
}
void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr ? Cast<AShooterCharacter>(GetOwner()) : Character;
	if(Character)
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		Package.ShooterCharacter = Character;
		for(auto& BoxPair : Character->HitCollisionBoxes)
		{
			FBoxInformation BoxInformation;
			BoxInformation.Location = BoxPair.Value->GetComponentLocation();
			BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
			Package.HitBoxInfo.Add(BoxPair.Key,BoxInformation);
		}
	}
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame,
	const FFramePackage& YoungerFrame, float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance,0.f,1.f);
	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;

	for (auto& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;
		
		const FBoxInformation& OlderBox = OlderFrame.HitBoxInfo[BoxInfoName];
		const FBoxInformation& YoungerBox = YoungerFrame.HitBoxInfo[BoxInfoName];

		FBoxInformation InterpBoxInfo;
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location,YoungerBox.Location,1.f,InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation,YoungerBox.Rotation,1.f,InterpFraction);
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;

		InterpFramePackage.HitBoxInfo.Add(BoxInfoName,InterpBoxInfo);
	}
	return InterpFramePackage;
}
FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package,
	AShooterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if(HitCharacter == nullptr) return FServerSideRewindResult();
	
	FFramePackage CurrentFrame;
	CacheBoxPosition(HitCharacter,CurrentFrame);
	MoveBoxes(HitCharacter,Package);
	EnableCharacterMeshCollision(HitCharacter,ECollisionEnabled::NoCollision);
	
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox,ECR_Block);
	
	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
	UWorld* World = GetWorld();
	if(World)
	{
		World->LineTraceSingleByChannel(ConfirmHitResult,TraceStart,TraceEnd,ECC_HitBox);
		if(ConfirmHitResult.bBlockingHit)//HeadShot
		{
			if(ConfirmHitResult.Component.IsValid())
			{
				UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
				if(Box)
				{
					DrawDebugBox(GetWorld(),Box->GetComponentLocation(),Box->GetScaledBoxExtent(),FQuat(Box->GetComponentRotation()),FColor::Red,false,8.f);
				}
			}
			ResetHitBoxes(HitCharacter,CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter,ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{true,true};
		}
		else //BodyShot
		{
			for(auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
			{
				if(HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox,ECR_Block);
				}
			}
			World->LineTraceSingleByChannel(ConfirmHitResult,TraceStart,TraceEnd,ECC_HitBox);
			if(ConfirmHitResult.bBlockingHit)
			{
				if(ConfirmHitResult.Component.IsValid())
				{
					UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
					if(Box)
					{
						DrawDebugBox(GetWorld(),Box->GetComponentLocation(),Box->GetScaledBoxExtent(),FQuat(Box->GetComponentRotation()),FColor::Black,false,8.f);
					}
				}
				ResetHitBoxes(HitCharacter,CurrentFrame);
				EnableCharacterMeshCollision(HitCharacter,ECollisionEnabled::QueryAndPhysics);
				return FServerSideRewindResult{true,false};
			}
		}
	}
	ResetHitBoxes(HitCharacter,CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter,ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{false,false};
}
FServerSideRewindResult ULagCompensationComponent::ProjectileConfirmHit(const FFramePackage& Package,AShooterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FFramePackage CurrentFrame;
	CacheBoxPosition(HitCharacter,CurrentFrame);
	MoveBoxes(HitCharacter,Package);
	EnableCharacterMeshCollision(HitCharacter,ECollisionEnabled::NoCollision);
	
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox,ECR_Block);

	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithCollision = true;
	PathParams.MaxSimTime = MaxRecordTime;
	PathParams.LaunchVelocity = InitialVelocity;
	PathParams.StartLocation = TraceStart;
	PathParams.SimFrequency = 15.f;
	PathParams.ProjectileRadius = 5.f;
	PathParams.TraceChannel = ECC_HitBox;
	PathParams.ActorsToIgnore.Add(GetOwner());
	PathParams.DrawDebugTime = 5.f;
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	FPredictProjectilePathResult PathResult;
	UGameplayStatics::PredictProjectilePath(this,PathParams,PathResult);
	
	if(PathResult.HitResult.bBlockingHit) /*Headshot hit return early */
	{
		if(PathResult.HitResult.Component.IsValid())
		{
			UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
			if(Box)
			{
				DrawDebugBox(GetWorld(),Box->GetComponentLocation(),Box->GetScaledBoxExtent(),FQuat(Box->GetComponentRotation()),FColor::Red,false,8.f);
			}
		}
		ResetHitBoxes(HitCharacter,CurrentFrame);
		EnableCharacterMeshCollision(HitCharacter,ECollisionEnabled::QueryAndPhysics);
		return FServerSideRewindResult{true,true};
	}
	else // No Headshot , body shot ? 
	{
		for(auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
		{
			if(HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox,ECR_Block);
			}
		}
		UGameplayStatics::PredictProjectilePath(this,PathParams,PathResult);
		if(PathResult.HitResult.bBlockingHit)
		{
			if(PathResult.HitResult.Component.IsValid())
			{
				UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
				if(Box)
				{DrawDebugBox(GetWorld(),Box->GetComponentLocation(),Box->GetScaledBoxExtent(),FQuat(Box->GetComponentRotation()),FColor::Black,false,8.f);
				}
			}
			ResetHitBoxes(HitCharacter,CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter,ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{true,false};
		}
	}
	ResetHitBoxes(HitCharacter,CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter,ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{false,false};
}
FShotGunServerSideRewind ULagCompensationComponent::ShotGunConfirmHit(const TArray<FFramePackage>& FramePackages,
                                                                      const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
	for(auto& Frame : FramePackages)
	{
		if(Frame.ShooterCharacter == nullptr) return FShotGunServerSideRewind();
	}
	FShotGunServerSideRewind ShotGunResult;
	TArray<FFramePackage> CurrentFrames;
	for(auto& Frame : FramePackages)
	{
		FFramePackage CurrentFrame;
		CurrentFrame.ShooterCharacter = Frame.ShooterCharacter;
		CacheBoxPosition(Frame.ShooterCharacter,CurrentFrame);
		MoveBoxes(Frame.ShooterCharacter,Frame);
		EnableCharacterMeshCollision(Frame.ShooterCharacter,ECollisionEnabled::NoCollision);
		CurrentFrames.Add(CurrentFrame);
	}
	for(auto& Frame : FramePackages)
	{
		UBoxComponent* HeadBox = Frame.ShooterCharacter->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox,ECR_Block);
	}
	UWorld* World = GetWorld();
	/*HeadShot*/
	for(auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		if(World)
		{
			World->LineTraceSingleByChannel(ConfirmHitResult,TraceStart,TraceEnd,ECC_HitBox);
			AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(ConfirmHitResult.GetActor());
			if(ShooterCharacter)
			{
				if(ConfirmHitResult.Component.IsValid())
				{
					UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
					if(Box)
					{
						DrawDebugBox(GetWorld(),Box->GetComponentLocation(),Box->GetScaledBoxExtent(),FQuat(Box->GetComponentRotation()),FColor::Red,false,8.f);
					}
				}
				if(ShotGunResult.HeadShots.Contains(ShooterCharacter))
				{
					ShotGunResult.HeadShots[ShooterCharacter]++;
				}
				else
				{
					ShotGunResult.HeadShots.Emplace(ShooterCharacter,1);
				}
			}
		}
	}
	/*Enable collision for all boxes and then disable for head box*/
	for(auto& Frame : FramePackages)
	{
		for(auto& HitBoxPair : Frame.ShooterCharacter->HitCollisionBoxes)
		{
			if(HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox,ECR_Block);
			}
		}
		UBoxComponent* HeadBox = Frame.ShooterCharacter->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	/*BodyShots?*/
	for(auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		if(World)
		{
			World->LineTraceSingleByChannel(ConfirmHitResult,TraceStart,TraceEnd,ECC_HitBox);
			AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(ConfirmHitResult.GetActor());
			if(ShooterCharacter)
			{
				if(ConfirmHitResult.Component.IsValid())
				{
					UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
					if(Box)
					{
						DrawDebugBox(GetWorld(),Box->GetComponentLocation(),Box->GetScaledBoxExtent(),FQuat(Box->GetComponentRotation()),FColor::Blue,false,8.f);
					}
				}
				if(ShotGunResult.BodyShots.Contains(ShooterCharacter))
				{
					ShotGunResult.BodyShots[ShooterCharacter]++;
				}
				else
				{
					ShotGunResult.BodyShots.Emplace(ShooterCharacter,1);
				}
			}
		}
	}
	for(auto& Frame : CurrentFrames)
	{
		ResetHitBoxes(Frame.ShooterCharacter,Frame);
		EnableCharacterMeshCollision(Frame.ShooterCharacter,ECollisionEnabled::QueryAndPhysics);
	}
	return ShotGunResult;
}
void ULagCompensationComponent::CacheBoxPosition(AShooterCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if(HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if(HitBoxPair.Value != nullptr)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key,BoxInfo);
		}
	}
}
void ULagCompensationComponent::MoveBoxes(AShooterCharacter* HitCharacter, const FFramePackage& Package)
{
	if(HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		
		if(HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
			}
	}
}
void ULagCompensationComponent::ResetHitBoxes(AShooterCharacter* HitCharacter, const FFramePackage& Package)
{
	if(HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if(HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}
void ULagCompensationComponent::EnableCharacterMeshCollision(AShooterCharacter* HitCharacter,
	ECollisionEnabled::Type CollisionEnabled)
{
	if(HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
	}
	
}
void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)
{
	for(auto& BoxInfo : Package.HitBoxInfo)
	{
		DrawDebugBox(GetWorld(),BoxInfo.Value.Location,BoxInfo.Value.BoxExtent,FQuat(BoxInfo.Value.Rotation),Color,false,4);
	}
}
FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(AShooterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter,HitTime);
	return ConfirmHit(FrameToCheck,HitCharacter,TraceStart,HitLocation);
}

FServerSideRewindResult ULagCompensationComponent::ProjectileServerSideRewind(AShooterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter,HitTime);
	return ProjectileConfirmHit(FrameToCheck,HitCharacter,TraceStart,InitialVelocity,HitTime);
}
FShotGunServerSideRewind ULagCompensationComponent::ShotGunServerSideRewind(
	const TArray<AShooterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	TArray<FFramePackage> FramesToCheck;
	for(AShooterCharacter* HitCharacter : HitCharacters)
	{
		FramesToCheck.Add(GetFrameToCheck(HitCharacter,HitTime));
	}
	return ShotGunConfirmHit(FramesToCheck,TraceStart,HitLocations);
}
FFramePackage ULagCompensationComponent::GetFrameToCheck(AShooterCharacter* HitCharacter, float HitTime)
{
	bool bReturn =
		HitCharacter == nullptr || HitCharacter->GetLagCompensation() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.GetHead() == nullptr ||HitCharacter->GetLagCompensation()->FrameHistory.GetTail() == nullptr;
	
	if(bReturn) return FFramePackage();

	/*Frame Package that gets checked to verify a hit*/
	FFramePackage FrameToCheck;
	
	bool bShouldInterpolate = true;
	/*Frame History of the Hit Character*/
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensation()->FrameHistory;
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;
	
	if(OldestHistoryTime > HitTime)
	{
		//To far back - too laggy to do ServerSideRewind
		return FFramePackage();
	}
	if(OldestHistoryTime == HitTime)
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterpolate = false;
	}
	if(NewestHistoryTime <= HitTime)
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpolate = false;
	}
	
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;

	while (Older->GetValue().Time > HitTime) // is older still younger than hittime ? 
	{
		// MarchBack until Older time < HitTime < YoungerTime;
		if(Older->GetNextNode() == nullptr) break;
		Older = Older->GetNextNode();
		if(Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}
	}
	if(Older->GetValue().Time == HitTime) // low chance!! :  but found frame to check
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}
	if(bShouldInterpolate)
	{
		/*Interpolation between Younger and Older*/
		FrameToCheck = InterpBetweenFrames(Older->GetValue(),Younger->GetValue(),HitTime);
	}
	FrameToCheck.ShooterCharacter = HitCharacter;
	return FrameToCheck;
}
void ULagCompensationComponent::ServerScoreRequest_Implementation(AShooterCharacter* HitCharacter,
                                                                  const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter,TraceStart,HitLocation,HitTime);
	if(Character && HitCharacter  && Confirm.bHitConfirmed && Character->GetEquippedWeapon())
	{
		const float Damage = Confirm.bHeadShot ? Character->GetEquippedWeapon()->GetHeadShotDamage() : Character->GetEquippedWeapon()->GetDamage();
		UGameplayStatics::ApplyDamage(HitCharacter,Damage,Character->Controller,Character->GetEquippedWeapon(),UDamageType::StaticClass());
	}
}
void ULagCompensationComponent::ProjectileServerScoreRequest_Implementation(AShooterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FServerSideRewindResult Confirm = ProjectileServerSideRewind(HitCharacter,TraceStart,InitialVelocity,HitTime);
	if(Character && HitCharacter && Confirm.bHitConfirmed && Character->GetEquippedWeapon())
	{
		const float Damage = Confirm.bHeadShot ? Character->GetEquippedWeapon()->GetHeadShotDamage() : Character->GetEquippedWeapon()->GetDamage();

		UGameplayStatics::ApplyDamage(HitCharacter,Damage,Character->Controller,Character->GetEquippedWeapon(),UDamageType::StaticClass());
	}
}
void ULagCompensationComponent::ShotgunServerScoreRequest_Implementation(
	const TArray<AShooterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	FShotGunServerSideRewind Confirm = ShotGunServerSideRewind(HitCharacters,TraceStart,HitLocations,HitTime);

	for(auto& HitCharacter : HitCharacters)
	{
		if(HitCharacter == nullptr || HitCharacter->GetEquippedWeapon() == nullptr || Character == nullptr) continue;
		float TotalDamage = 0.f;
		if(Confirm.HeadShots.Contains(HitCharacter))
		{
			float HeadShotDamage = Confirm.HeadShots[HitCharacter] * HitCharacter->GetEquippedWeapon()->GetHeadShotDamage();
			TotalDamage += HeadShotDamage;
		}
		if(Confirm.BodyShots.Contains(HitCharacter))
		{
			float BodyShotDamage = Confirm.BodyShots[HitCharacter] * HitCharacter->GetEquippedWeapon()->GetDamage();
			TotalDamage += BodyShotDamage;
		}
		UGameplayStatics::ApplyDamage(HitCharacter,TotalDamage,Character->Controller,HitCharacter->GetEquippedWeapon(),UDamageType::StaticClass());
	}
}

