// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine.h"
#include "TrafficManager.generated.h"

UCLASS()
class TGK_API ATrafficManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATrafficManager();

	UPROPERTY(EditAnywhere)
	int numberOfLanes = 2;

	UPROPERTY(EditAnywhere)
	int numberOfCarsActive = 0;

	//UPROPERTY(EditAnywhere)
	//TArray<class ATrafficLane*> trafficLanesArray;
	UPROPERTY(EditAnywhere)
	TArray<class USplineComponent*> trafficSplinesArray;
	UPROPERTY(EditAnywhere)
	TArray<int> numberOfCarsPerTrafficLane;

	int trafficLaneIndexToSpawn = 0;
	int trafficLaneIndexToDestroy = 0;

	UPROPERTY(EditAnywhere)
	int maxNumberOfCarsActive = 1;

	UPROPERTY(EditAnywhere)
	TArray<class ATrafficCarPawn*> activeCarsInTrafficLane0;
	UPROPERTY(EditAnywhere)
	TArray<class ATrafficCarPawn*> activeCarsInTrafficLane1;
	UPROPERTY(EditAnywhere)
	TArray<class ATrafficCarPawn*> activeCarsInTrafficLane2;
	UPROPERTY(EditAnywhere)
	TArray<class ATrafficCarPawn*> activeCars;

	bool canSpawnCars = false;
	//void SpawnCar(class ATrafficLane* trafficLaneToSpawnOn);
	//void SpawnCar();

	//void DestroyCar(class ATrafficCarPawn* carToDestroy, class ATrafficLane* trafficLaneToDestroyFrom);
	//void DestroyCar(class ATrafficLane* trafficLaneToDestroyFrom);

	//class ATrafficLane* GetLeastPopulatedLane();
	//class ATrafficLane* GetMostPopulatedLane();
	
	bool isReadyToSpawn = true;
	float timeUntilNextCarSpawn = 300.f;
	float maxTimeUntilNextCarSpawn = 300.f;

	float baseOffset = 10000.f;
	float spacer = 1000.f;
	bool space = false;
	float allowedMaxDistanceBehind = 650.f;

	UPROPERTY(EditAnywhere)
	class APlayerCarPawn* playerCar;

	//float FindDistanceAlongSplineClosestToLocation(class ATrafficLane* trafficLaneToSpawnOn);

	//void AddTrafficCarPawnToArray(int index, class ATrafficCarPawn* carToAdd);
	//void RemoveTrafficCarPawnFromArray(int index, class ATrafficCarPawn* carToRemove);

	//void CheckIfExistingCarsAreBehindPlayer();

	UBoxComponent* spawnCollisionVolume;

	bool SpawnDestinationCheckOverlap();

	bool isReadyToDelete = false;
	float timeUntilCheckForDelete = 30.f;
	float maxTimeUntilCheckForDelete = 30.f;

	bool hasDoneInitialSpawn = false;
	//void SpawnCarInitial();

	float initialCarSpawnDistance = 3000.f;
	float initialCarSpawnSameGroupDistance = 0.f;
	float initialCarSpawnSeperateGroupDistance = 0.f;

	UPROPERTY(EditAnywhere)
	float separateGroupDistanceMin = 4500.f;

	UPROPERTY(EditAnywhere)
	float separateGroupDistanceMax = 6500.f;

	int previousLane = -1;

	int groupSize = 0;
	int carsInThisGroup = 0;

	UPROPERTY(EditAnywhere)
	bool isTrafficActive = false;

	UPROPERTY(EditAnywhere)
	class AGameManager* gameManager;

	void CarHasBeenPassed(class ATrafficCarPawn* carPassed);
	void DoInitialCarSpawn();
	void DoSingleCarSpawn();
	void DoPairCarSpawn();
	void DoCarDestroy(class ATrafficCarPawn* carToDestroy);

	float FindDistanceAlongSplineFromOffset(class USplineComponent* trafficLaneToSpawnOn);

	bool spawningAPair = false;
	UPROPERTY(EditAnywhere)
	float spawnOffsetDistance = 10000;

	UPROPERTY(EditAnywhere)
	class ATrack* track;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
