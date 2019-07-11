// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "Classes/Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SplineComponent.h"
#include "TrafficCarPawn.generated.h"

UCLASS()
class TGK_API ATrafficCarPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ATrafficCarPawn();

	UPROPERTY(EditAnywhere)
	USkeletalMeshComponent* trafficCarMesh;

	UPROPERTY(EditAnywhere)
	USphereComponent* movementGuideSphere;

	UPROPERTY(EditAnywhere)
	class ATrafficManager* trafficManager;

	float movementGuideSphereInitialDistanceOnSpline = 0.f;
	float movementGuideSphereInitialOffset = 1200.f;
	float movementGuideSphereAdditionalDistance = 0.f;

	float totalDistanceTraveled = 0;

	USplineComponent* trafficLaneSpine;

	bool isRecovering = false;
	float recoverTime = 180;
	float recoverMaxTime = 180;

	bool isFindingUpright = false;

	bool isMarkedForDeletion = false;

	bool isReady = false;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* comp, AActor* otherActor, UPrimitiveComponent* otherComp, FVector ahsjfghaskf, const FHitResult &asgasgasgasha);

	int numberInQueue = -1;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* playerCarOverlapVolume;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
