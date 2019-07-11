// Fill out your copyright notice in the Description page of Project Settings.

#include "PowerSource.h"


// Sets default values
APowerSource::APowerSource()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	overlapVolume = CreateDefaultSubobject<UBoxComponent>("Overlap volume");
	SetRootComponent(overlapVolume);
	overlapVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	overlapVolume->SetCollisionObjectType(ECC_GameTraceChannel6);
	overlapVolume->SetCollisionResponseToChannel(ECC_GameTraceChannel6, ECollisionResponse::ECR_Overlap);
	overlapVolume->bGenerateOverlapEvents = true;
	overlapVolume->SetHiddenInGame(false);

	activeParticles = CreateDefaultSubobject<UParticleSystem>("Active particles");
	static ConstructorHelpers::FObjectFinder<UParticleSystem> particleSystemObject2(TEXT("/Game/InfinityBladeEffects/Effects/FX_Ambient/Lighting/P_FakeLightBlue_mobile"));
	activeParticles = particleSystemObject2.Object;

	restoreParticles = CreateDefaultSubobject<UParticleSystem>("Restore particles");
	static ConstructorHelpers::FObjectFinder<UParticleSystem> particleSystemObject1(TEXT("/Game/InfinityBladeEffects/Effects/FX_Skill_Aura/P_Aura_Default_Upheaval_01"));
	restoreParticles = particleSystemObject1.Object;
}

// Called when the game starts or when spawned
void APowerSource::BeginPlay()
{
	Super::BeginPlay();

	activeParticleComponent = UGameplayStatics::SpawnEmitterAttached(activeParticles, overlapVolume);

	overlapVolume->OnComponentBeginOverlap.AddDynamic(this, &APowerSource::OnPowerSourceOverlapBegin);
	overlapVolume->OnComponentEndOverlap.AddDynamic(this, &APowerSource::OnPowerSourceOverlapEnd);
}

// Called every frame
void APowerSource::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

UFUNCTION()
void APowerSource::OnPowerSourceOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (active)
	{
		activeParticleComponent->DestroyComponent();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), restoreParticles, overlapVolume->GetComponentTransform(), true);
	}
}

UFUNCTION()
void APowerSource::OnPowerSourceOverlapEnd(UPrimitiveComponent* overlappedComp, AActor* otherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

