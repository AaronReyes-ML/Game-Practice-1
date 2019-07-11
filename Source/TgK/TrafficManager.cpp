// Fill out your copyright notice in the Description page of Project Settings.

#include "TrafficManager.h"
#include "TrafficLane.h"
#include "TrafficCarPawn.h"
#include "GameManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "PlayerCarPawn.h"
#include "Track.h"


// Sets default values
ATrafficManager::ATrafficManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	spawnCollisionVolume = CreateDefaultSubobject<UBoxComponent>("Spawn collision volume");
	spawnCollisionVolume->SetCollisionObjectType(ECC_WorldDynamic);
	spawnCollisionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	spawnCollisionVolume->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	spawnCollisionVolume->SetBoxExtent(FVector(245.f, 100.f, 70.f));
	spawnCollisionVolume->SetHiddenInGame(true);
	spawnCollisionVolume->bGenerateOverlapEvents = false;
}

// Called when the game starts or when spawned
void ATrafficManager::BeginPlay()
{
	Super::BeginPlay();

	trafficSplinesArray.Add(track->lane1);
	trafficSplinesArray.Add(track->lane2);

	//DoInitialCarSpawn();
	isTrafficActive = true;
}

// Called every frame
void ATrafficManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (track)
	{
		if (!hasDoneInitialSpawn)
		{
			DoInitialCarSpawn();
		}
	}

	/*
	if (!hasDoneInitialSpawn)
	{
		if (groupSize == 0)
		{
			groupSize = FMath::RandRange(1, 3);
		}
		else if (carsInThisGroup == groupSize)
		{
			groupSize = FMath::RandRange(1, 3);
			carsInThisGroup = 0;
		}

		if (numberOfCarsActive < maxNumberOfCarsActive)
		{
			SpawnCarInitial();
		}
		else
		{
			carsInThisGroup = 0;
			previousLane = -1;
			hasDoneInitialSpawn = true;
			isTrafficActive = true;
		}
	}
	else
	{
		if (isReadyToSpawn && isReadyToDelete)
		{
			CheckIfExistingCarsAreBehindPlayer();
			isReadyToDelete = false;
		}
		else
		{
			if (timeUntilCheckForDelete > 0)
			{
				timeUntilCheckForDelete -= 1;
			}
			else
			{
				timeUntilCheckForDelete = maxTimeUntilCheckForDelete;
				isReadyToDelete = true;
			}
		}

		if (isReadyToSpawn && numberOfCarsActive < maxNumberOfCarsActive)
		{
			if (groupSize == 0)
			{
				groupSize = FMath::RandRange(1, 3);
			}
			else if (carsInThisGroup == groupSize)
			{
				groupSize = FMath::RandRange(1, 3);
				carsInThisGroup = 0;
			}

			SpawnCar();
			isReadyToSpawn = false;
		}
		else
		{
			if (timeUntilNextCarSpawn > 0)
			{
				timeUntilNextCarSpawn -= 1;
			}
			else
			{
				isReadyToSpawn = true;
				timeUntilNextCarSpawn = maxTimeUntilNextCarSpawn;
			}
		}
	}
	*/

}

bool ATrafficManager::SpawnDestinationCheckOverlap()
{
	// Get all overlappingActors
	TArray<AActor*> overlappingActors;
	overlappingActors.Reset();
	spawnCollisionVolume->GetOverlappingActors(overlappingActors);

	// If there are any overlappingActors, this is an invalid destination
	if (overlappingActors.Num() != 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void ATrafficManager::DoInitialCarSpawn()
{
	//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::White, FString("Attempting to do initial spawn"));
	if (numberOfCarsActive < maxNumberOfCarsActive)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::White, FString("Active cars less than max"));
		//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::White, FString::SanitizeFloat(numberOfCarsActive));
		//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::White, FString::SanitizeFloat(maxNumberOfCarsActive));
		DoSingleCarSpawn();
	}
	else
	{
		hasDoneInitialSpawn = true;
	}
}

void ATrafficManager::DoSingleCarSpawn()
{
	//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::White, FString("Attempting to do single car spawn"));
	int randomLane = FMath::RandRange(0, numberOfLanes - 1);

	//ATrafficLane* trafficLaneToSpawnOn = trafficLanesArray[randomLane];
	USplineComponent* trafficLaneToSpawnOn = trafficSplinesArray[randomLane];
	float distanceOnSplineToSpawn = FindDistanceAlongSplineFromOffset(trafficLaneToSpawnOn);

	FRotator rotationToSpawnCar = trafficLaneToSpawnOn->GetRotationAtDistanceAlongSpline(distanceOnSplineToSpawn, ESplineCoordinateSpace::World);
	FVector locationToSpawnCar = trafficLaneToSpawnOn->GetLocationAtDistanceAlongSpline(distanceOnSplineToSpawn, ESplineCoordinateSpace::World) + -FVector(0.f, 0.f, 1.f) * 70;

	spawnCollisionVolume->bGenerateOverlapEvents = true;
	spawnCollisionVolume->SetWorldLocation(locationToSpawnCar);
	spawnCollisionVolume->SetWorldRotation(rotationToSpawnCar);

	if (distanceOnSplineToSpawn < trafficLaneToSpawnOn->GetSplineLength())
	{
		if (!SpawnDestinationCheckOverlap())
		{
			//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::White, FString("OVERLAP PASSED"));
			if (GetWorld())
			{
				ATrafficCarPawn* spawnedCar = GetWorld()->SpawnActor<ATrafficCarPawn>(ATrafficCarPawn::StaticClass(), locationToSpawnCar, rotationToSpawnCar);
				if (spawnedCar != NULL)
				{
					//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::White, FString("SPAWNED CAR NOT NULL"));
					spawnedCar->trafficManager = this;
					spawnedCar->trafficLaneSpine = trafficLaneToSpawnOn;
					spawnedCar->trafficCarMesh->SetAllBodiesNotifyRigidBodyCollision(true);

					activeCars.Add(spawnedCar);

					FVector locationToStartMovementGuideSphere = trafficLaneToSpawnOn->GetLocationAtDistanceAlongSpline(distanceOnSplineToSpawn + spawnedCar->movementGuideSphereInitialOffset, ESplineCoordinateSpace::World);

					spawnedCar->trafficCarMesh->SetWorldLocationAndRotation(locationToSpawnCar, rotationToSpawnCar, false, (FHitResult *)nullptr, ETeleportType::TeleportPhysics);
					spawnedCar->movementGuideSphere->SetWorldLocation(locationToStartMovementGuideSphere);
					spawnedCar->movementGuideSphereInitialDistanceOnSpline = distanceOnSplineToSpawn + spawnedCar->movementGuideSphereInitialOffset;

					numberOfCarsActive += 1;
				}
				else
				{
					//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::White, FString("SPAWNED CAR IS NULL"));
				}
			}
			else
			{
				//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::White, FString("WORLD IS NULL"));
			}
		}
		else
		{
			//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::White, FString("OVERLAP FAILED"));
		}
	}

	spawnCollisionVolume->bGenerateOverlapEvents = false;
}

float ATrafficManager::FindDistanceAlongSplineFromOffset(USplineComponent* trafficLaneToSpawnOn)
{
	float furthestCarDistanceOnCourseSpline = 0;
	if (numberOfCarsActive == 0)
	{
		furthestCarDistanceOnCourseSpline = gameManager->GetDistanceTraveled();
	}
	else
	{
		furthestCarDistanceOnCourseSpline = gameManager->GetDistanceTraveled(activeCars[numberOfCarsActive-1]->trafficCarMesh->GetComponentLocation());
	}
	float offsetIncludedDistance = furthestCarDistanceOnCourseSpline + (spawnOffsetDistance * FMath::RandRange(1,2.5));
	FVector worldLocationAtOffsetIncludedDistance = gameManager->courseSpline->GetLocationAtDistanceAlongSpline(offsetIncludedDistance, ESplineCoordinateSpace::World);

	USplineComponent* trafficLaneSpline = trafficLaneToSpawnOn;

	float inputKey = trafficLaneSpline->FindInputKeyClosestToWorldLocation(worldLocationAtOffsetIncludedDistance);

	int inputKeyTrunc1 = FMath::TruncToInt(inputKey);
	int inputKeyTrunc2 = FMath::TruncToInt(inputKey + 1.f);

	float distanceOnSpline1 = trafficLaneSpline->GetDistanceAlongSplineAtSplinePoint(inputKeyTrunc1);
	float distanceOnSpline2 = trafficLaneSpline->GetDistanceAlongSplineAtSplinePoint(inputKeyTrunc2);

	float distanceDelta = distanceOnSpline2 - distanceOnSpline1;
	float keyDelta = inputKey - (float)inputKeyTrunc1;

	float distanceToPoint2 = distanceDelta * keyDelta;

	float finalDistance = distanceOnSpline1 + distanceToPoint2;

	return finalDistance;
}

void ATrafficManager::DoPairCarSpawn()
{

}

void ATrafficManager::CarHasBeenPassed(ATrafficCarPawn* carPassed)
{
	bool doDelete = false;
	for (int i = 0; i < activeCars.Num(); i++)
	{
		if (activeCars[i] == carPassed)
		{
			if (i >= 1)
			{
				doDelete = true;
				break;
			}
		}
	}

	if (doDelete)
	{
		ATrafficCarPawn* carToDestroy = activeCars[0];
		DoCarDestroy(carToDestroy);
		DoSingleCarSpawn();
	}

}

void ATrafficManager::DoCarDestroy(ATrafficCarPawn* carToDestroy)
{
	activeCars.Remove(carToDestroy);
	activeCars.Shrink();
	numberOfCarsActive -= 1;
	carToDestroy->Destroy();
}
