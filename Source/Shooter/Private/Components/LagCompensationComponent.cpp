// Hello :) 


#include "Components/LagCompensationComponent.h"
#include "Character/ShooterCharacter.h"
#include "Components/BoxComponent.h"

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

		ShowFramePackage(ThisFrame,FColor::Red);
	}

}
void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr ? Cast<AShooterCharacter>(GetOwner()) : Character;
	if(Character)
	{
		Package.Time = GetWorld()->GetTimeSeconds();
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

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)
{
	for(auto& BoxInfo : Package.HitBoxInfo)
	{
		DrawDebugBox(GetWorld(),BoxInfo.Value.Location,BoxInfo.Value.BoxExtent,FQuat(BoxInfo.Value.Rotation),Color,false,4);
	}
}
void ULagCompensationComponent::ServerSideRewind(AShooterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation, float HitTime)
{
	bool bReturn =
		HitCharacter == nullptr || HitCharacter->GetLagCompensation() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.GetHead() == nullptr ||HitCharacter->GetLagCompensation()->FrameHistory.GetTail() == nullptr;

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
		return;
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
	}
	if(bReturn) return;
	
}


