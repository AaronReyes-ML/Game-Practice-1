// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Classes/Components/BoxComponent.h"
#include "Classes/Components/AudioComponent.h"
#include "Runtime/Engine/Classes/Sound/SoundCue.h"
#include "Components/SplineComponent.h"
#include "GameManager.generated.h"

UCLASS()
class TGK_API AGameManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGameManager();

	UPROPERTY(EditAnywhere)
	class USceneComponent* rootComp;

	UPROPERTY(EditAnywhere)
	UBoxComponent* levelEndOverlapComponent;

	UPROPERTY(EditAnywhere)
	class APlayerCarPawn* playerCar;

	UPROPERTY(EditAnywhere)
	UAudioComponent* musicAudioComponent;

	UPROPERTY(EditAnywhere)
	USoundCue* levelSoundCue;

	UPROPERTY(EditAnywhere)
	USoundCue* preLevelSoundCue;

	UPROPERTY(EditAnywhere)
	USoundCue* postLevelSoundCue;

	void PlayPreLevelSoundCue();
	void PlayLevelSoundCue();
	void PlayPostLevelSoundCue();
	void PauseMusic();
	void UnPauseMusic();

	UPROPERTY(EditAnywhere)
	USplineComponent* courseSpline;

	float GetPlayerDistanceFromFinish();
	float GetPercentDone();
	float GetDistanceTraveled();
	float GetDistanceTraveled(FVector location);

	float distanceTraveled = 0;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere)
	class ATrack* track;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
