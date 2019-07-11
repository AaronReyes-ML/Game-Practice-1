// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Track.generated.h"

USTRUCT()
struct FTrackSegmentStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
		float TrackBank;
	UPROPERTY(EditAnywhere)
		float TrackWidth;
	UPROPERTY(EditAnywhere)
		float TrackDepth;

	FTrackSegmentStruct()
	{
		TrackBank = 0;
		TrackWidth = 10.f;
		TrackDepth = 1.f;
	}
};

UCLASS()
class TGK_API ATrack : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATrack();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void OnConstruction(const FTransform& transform) override;

	UPROPERTY(EditAnywhere)
	bool construct = true;
	UPROPERTY(EditAnywhere)
	bool collisionEnabled = false;

	UFUNCTION()
	void DoTrackBuild();

	UFUNCTION()
	void BuildTrackSegment(int32 currentLoopIndex);

	UFUNCTION()
	void UpdateTrackSegmentArray(int32 index);
	
	UPROPERTY(EditAnywhere)
	int32 numberOfCoreSplinePoints = 0;

	UPROPERTY(EditAnywhere)
	TArray<FTrackSegmentStruct> TrackSegmentArray;

	UPROPERTY(EditAnywhere)
	class UStaticMesh* trackMesh;
	
	UPROPERTY(EditAnywhere)
	class USplineComponent* trackCoreSpline;

	UPROPERTY(EditAnywhere)
	class USplineComponent* lane1;

	UPROPERTY(EditAnywhere)
	class USplineComponent* lane2;

	UPROPERTY(EditAnywhere)
	TArray<class USplineComponent*> trafficSplineArray;

	UPROPERTY(EditAnywhere)
	int32 numberOfTrafficLanes = 2;
};
