// Hello :) 


#include "PickUp/PickUp.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

APickUps::APickUps()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(RootComponent);
	OverlapSphere->SetSphereRadius(85.f);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
	OverlapSphere->AddLocalOffset(FVector(0.f,0.f,85.f));

	PickUpMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickUpMesh"));
	PickUpMesh->SetupAttachment(OverlapSphere);
	PickUpMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickUpMesh->SetRelativeScale3D(FVector(2.5f,2.5f,2.5f));
	PickUpMesh->SetRenderCustomDepth(true);
	PickUpMesh->SetCustomDepthStencilValue(250);

	PickupEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickUpEffectComponent"));
	PickupEffectComponent->SetupAttachment(RootComponent);

}
void APickUps::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		GetWorldTimerManager().SetTimer(BindOverlapTimer,this,&APickUps::BindOverlapTimerFinished,BindOverlapTime);
	}
	
	
}
void APickUps::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if(PickUpMesh)
	{
		PickUpMesh->AddWorldRotation(FRotator(0.f,BaseTurnRate * DeltaTime,false));
	}

}
void APickUps::Destroyed()
{
	Super::Destroyed();
	if(PickUpSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this,PickUpSound,GetActorLocation());
	}
	if(PickUpEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this,PickUpEffect,GetActorLocation(),GetActorRotation());
	}
}
void APickUps::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,
                               bool bFromSweep,const FHitResult& SweepResult)
{
	
}
void APickUps::BindOverlapTimerFinished()
{
	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this,&APickUps::OnSphereOverlap);
}

