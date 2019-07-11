// Fill out your copyright notice in the Description page of Project Settings.

#include "TrafficLane.h"


// Sets default values
ATrafficLane::ATrafficLane()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ATrafficLane::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATrafficLane::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

