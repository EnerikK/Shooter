// Hello :) 


#include "Weapon/AmmoEject.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


AAmmoEject::AAmmoEject()
{
	PrimaryActorTick.bCanEverTick = false;

	Ammo = CreateDefaultSubobject<UStaticMeshComponent>("Ammo");
	SetRootComponent(Ammo);
	Ammo->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	Ammo->SetSimulatePhysics(true);
	Ammo->SetEnableGravity(true);
	Ammo->SetNotifyRigidBodyCollision(true);
	ShellEjectionImpulse = 10.f;
	

}

void AAmmoEject::BeginPlay()
{
	Super::BeginPlay();

	Ammo->OnComponentHit.AddDynamic(this,&AAmmoEject::OnHit);
	Ammo->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);
	
}

void AAmmoEject::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if(ShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this,ShellSound,GetActorLocation());
	}
	Destroy();
}

