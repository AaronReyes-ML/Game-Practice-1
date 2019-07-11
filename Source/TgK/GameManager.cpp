// Fill out your copyright notice in the Description page of Project Settings.

#include "GameManager.h"
#include "PlayerCarPawn.h"
#include "Track.h"


// Sets default values
AGameManager::AGameManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	rootComp = CreateDefaultSubobject<USceneComponent>("Root");

	levelEndOverlapComponent = CreateDefaultSubobject<UBoxComponent>("Level end overlap component");
	levelEndOverlapComponent->SetWorldLocation(FVector(0.f, 0.f, 0.f));
	levelEndOverlapComponent->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	levelEndOverlapComponent->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel7);
	levelEndOverlapComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	levelEndOverlapComponent->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Overlap);
	musicAudioComponent = CreateDefaultSubobject<UAudioComponent>("Music audio component");

}

// Called when the game starts or when spawned
void AGameManager::BeginPlay()
{
	Super::BeginPlay();

	courseSpline = track->trackCoreSpline;

	levelEndOverlapComponent->OnComponentBeginOverlap.AddDynamic(this, &AGameManager::OnOverlapBegin);
}

// Called every frame
void AGameManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGameManager::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->GetClass() == APlayerCarPawn::StaticClass())
	{
		playerCar->DoLevelExit();
	}
}

void AGameManager::PlayPreLevelSoundCue()
{
	if (preLevelSoundCue != NULL)
	{
		musicAudioComponent->SetSound(preLevelSoundCue);
		musicAudioComponent->Play();
	}
}

void AGameManager::PlayLevelSoundCue()
{
	if (levelSoundCue != NULL)
	{
		musicAudioComponent->SetSound(levelSoundCue);
		musicAudioComponent->Play();
	}
}

void AGameManager::PlayPostLevelSoundCue()
{
	if (postLevelSoundCue != NULL)
	{
		musicAudioComponent->SetSound(postLevelSoundCue);
		musicAudioComponent->Play();
	}
}

float AGameManager::GetPlayerDistanceFromFinish()
{
	return (courseSpline->GetSplineLength() - distanceTraveled) / 100;
}

float AGameManager::GetPercentDone()
{
	return distanceTraveled / courseSpline->GetSplineLength();
}

float AGameManager::GetDistanceTraveled()
{
	if (playerCar)
	{
		float inputKey = courseSpline->FindInputKeyClosestToWorldLocation(playerCar->CarMesh->GetComponentLocation());

		int inputKeyTrunc1 = FMath::TruncToInt(inputKey);
		int inputKeyTrunc2 = FMath::TruncToInt(inputKey + 1.f);

		float distanceOnSpline1 = courseSpline->GetDistanceAlongSplineAtSplinePoint(inputKeyTrunc1);
		float distanceOnSpline2 = courseSpline->GetDistanceAlongSplineAtSplinePoint(inputKeyTrunc2);

		float distanceDelta = distanceOnSpline2 - distanceOnSpline1;
		float keyDelta = inputKey - (float)inputKeyTrunc1;

		float distanceToPoint2 = distanceDelta * keyDelta;

		distanceTraveled = distanceOnSpline1 + distanceToPoint2;
		return distanceTraveled;
	}
	return -1;
}

float AGameManager::GetDistanceTraveled(FVector location)
{
	float inputKey = courseSpline->FindInputKeyClosestToWorldLocation(location);

	int inputKeyTrunc1 = FMath::TruncToInt(inputKey);
	int inputKeyTrunc2 = FMath::TruncToInt(inputKey + 1.f);

	float distanceOnSpline1 = courseSpline->GetDistanceAlongSplineAtSplinePoint(inputKeyTrunc1);
	float distanceOnSpline2 = courseSpline->GetDistanceAlongSplineAtSplinePoint(inputKeyTrunc2);

	float distanceDelta = distanceOnSpline2 - distanceOnSpline1;
	float keyDelta = inputKey - (float)inputKeyTrunc1;

	float distanceToPoint2 = distanceDelta * keyDelta;

	distanceTraveled = distanceOnSpline1 + distanceToPoint2;
	return distanceTraveled;
}

void AGameManager::PauseMusic()
{
	if (musicAudioComponent->IsPlaying())
	{
		musicAudioComponent->SetPaused(true);
	}

}

void AGameManager::UnPauseMusic()
{
	if (!musicAudioComponent->IsPlaying())
	{
		musicAudioComponent->SetPaused(false);
	}
}