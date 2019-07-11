// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystemComponent.h"
#include "ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "PowerSource.generated.h"

UCLASS()
class TGK_API APowerSource : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APowerSource();

	UPROPERTY(EditAnywhere)
	UBoxComponent* overlapVolume;

	UPROPERTY(EditAnywhere)
	float restoreAmount = 600.f;

	UPROPERTY(EditAnywhere)
	float healAmount = 20.f;

	bool active = true;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnPowerSourceOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnPowerSourceOverlapEnd(UPrimitiveComponent* overlappedComp, AActor* otherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UParticleSystem* activeParticles;
	UParticleSystem* restoreParticles;

	UParticleSystemComponent* activeParticleComponent;
	
};
