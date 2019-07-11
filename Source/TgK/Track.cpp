// Fill out your copyright notice in the Description page of Project Settings.

#include "Track.h"
#include "Classes/Engine/StaticMesh.h"
#include "Classes/Components/SplineComponent.h"
#include "Classes/Components/SplineMeshComponent.h"
#include "ConstructorHelpers.h"


// Sets default values
ATrack::ATrack()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	trackCoreSpline = CreateDefaultSubobject<USplineComponent>("Track Core Spline");
	numberOfCoreSplinePoints = trackCoreSpline->GetNumberOfSplinePoints();
	this->SetRootComponent(trackCoreSpline);

	trackMesh = CreateDefaultSubobject<UStaticMesh>("TrackMeshBase");
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT("/Game/TgR_Content/Meshes/RoadMesh"));
	trackMesh = MeshAsset.Object;

	lane1 = CreateDefaultSubobject<USplineComponent>("Lane 1");
	lane1->AttachToComponent(trackCoreSpline, FAttachmentTransformRules::SnapToTargetIncludingScale);

	lane2 = CreateDefaultSubobject<USplineComponent>("Lane 2");
	lane2->AttachToComponent(trackCoreSpline, FAttachmentTransformRules::SnapToTargetIncludingScale);

	trafficSplineArray.Add(lane1);
	trafficSplineArray.Add(lane2);

}

// Called when the game starts or when spawned
void ATrack::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATrack::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called when the spline is modified
// Used to draw new spline mesh segments for track surface
void ATrack::OnConstruction(const FTransform& transform)
{
	//Super::OnConstruction(transform);

	// Reset number of total spline points on main spline
	numberOfCoreSplinePoints = trackCoreSpline->GetNumberOfSplinePoints();
	
	if (construct)
	{
		for (int i = 0; i < numberOfTrafficLanes; i++)
		{
			while (trafficSplineArray[i]->GetNumberOfSplinePoints() < numberOfCoreSplinePoints)
			{
				trafficSplineArray[i]->AddSplinePoint(FVector::ZeroVector, ESplineCoordinateSpace::World);
			}
		}
	}

	if (TrackSegmentArray.Num() < numberOfCoreSplinePoints)
	{
		while (TrackSegmentArray.Num() < numberOfCoreSplinePoints)
		{
			FTrackSegmentStruct newSegment = FTrackSegmentStruct();
			TrackSegmentArray.Add(newSegment);
		}
	}
	else if (TrackSegmentArray.Num() > numberOfCoreSplinePoints)
	{
		while (TrackSegmentArray.Num() > numberOfCoreSplinePoints)
		{
			TrackSegmentArray.RemoveAt(TrackSegmentArray.Num() - 1);
		}
	}

	// Builds the track mesh
	if (TrackSegmentArray.Num() > 0)
	{
		DoTrackBuild();
	}
}

// For each spline point, build its track segment
void ATrack::DoTrackBuild()
{
	for (int32 i = 0; i < numberOfCoreSplinePoints - 1; i++)
	{
		BuildTrackSegment(i);
	}
}

void ATrack::UpdateTrackSegmentArray(int32 index)
{
	//TrackSegmentArray[index].TrackWidth = trackCoreSpline->GetScaleAtSplinePoint(index).X;
	//TrackSegmentArray[index].TrackDepth = trackCoreSpline->GetScaleAtSplinePoint(index).Y;
	TrackSegmentArray[index].TrackBank = trackCoreSpline->GetRollAtSplinePoint(index, ESplineCoordinateSpace::World);
}

// Builds a spline mesh component from one spline point to the next spline point in the spline
void ATrack::BuildTrackSegment(int32 currentLoopIndex)
{
	// The next spline point in sequence, wraps around to 0 if exceeds number of spline points
	int32 nextLoopIndex = (currentLoopIndex + (int32)1) % numberOfCoreSplinePoints;

	UpdateTrackSegmentArray(currentLoopIndex);
	UpdateTrackSegmentArray(nextLoopIndex);

	// Store the scale and roll value for the start of the spline mesh
	FVector2D startScale;
	startScale.Set(TrackSegmentArray[currentLoopIndex].TrackWidth, TrackSegmentArray[currentLoopIndex].TrackDepth);
	float startRoll = TrackSegmentArray[currentLoopIndex].TrackBank;

	// Store the scale and roll value for the end of the spline mesh
	FVector2D endScale;
	endScale.Set(TrackSegmentArray[nextLoopIndex].TrackWidth, TrackSegmentArray[nextLoopIndex].TrackDepth);
	float endRoll = TrackSegmentArray[nextLoopIndex].TrackBank;

	// Store the start location and tangent of spline mesh component
	FVector startLocation;
	FVector startTangent;
	trackCoreSpline->GetLocationAndTangentAtSplinePoint(currentLoopIndex, startLocation, startTangent, ESplineCoordinateSpace::World);

	// Store the end location and tangent of the spline mesh component
	FVector endLocation;
	FVector endTangent;
	trackCoreSpline->GetLocationAndTangentAtSplinePoint(nextLoopIndex, endLocation, endTangent, ESplineCoordinateSpace::World);

	// Create the new spline mesh component
	USplineMeshComponent* newSplineMeshComponent = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
	newSplineMeshComponent->SetStaticMesh(trackMesh);
	newSplineMeshComponent->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	RegisterAllComponents();

	// Set the location, tangent, scale, and roll values for the spline mesh component
	newSplineMeshComponent->SetStartAndEnd(startLocation, startTangent, endLocation, endTangent);
	newSplineMeshComponent->SetStartScale(startScale);
	newSplineMeshComponent->SetEndScale(endScale);
	newSplineMeshComponent->SetStartRoll(startRoll);
	newSplineMeshComponent->SetEndRoll(endRoll);
	if (collisionEnabled)
	{
		newSplineMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		newSplineMeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
		newSplineMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	}
	// Attach the spline mesh component to the track core spline
	newSplineMeshComponent->AttachToComponent(trackCoreSpline, FAttachmentTransformRules::SnapToTargetIncludingScale);

	if (construct)
	{
		for (int i = 0; i < numberOfTrafficLanes; i++)
		{

			float shiftCurrent = (TrackSegmentArray[currentLoopIndex].TrackWidth / (numberOfTrafficLanes + 2)) * 100;
			float shitNext = (TrackSegmentArray[nextLoopIndex].TrackWidth / (numberOfTrafficLanes + 2)) * 100;
			float currentMultiplier = 1;
			float nextMultiplier = -1;

			FVector firstLaneDirection = startTangent.RotateAngleAxis(-90.f, FVector(0, 0, 1));
			FVector secondLaneDirection = endTangent.RotateAngleAxis(90.f, FVector(0, 0, 1));

			firstLaneDirection.Normalize();
			secondLaneDirection.Normalize();

			if (i == 0)
			{
				trafficSplineArray[i]->SetLocationAtSplinePoint(currentLoopIndex, startLocation + firstLaneDirection * shiftCurrent + FVector(0, 0, 500), ESplineCoordinateSpace::World);
				trafficSplineArray[i]->SetLocationAtSplinePoint(nextLoopIndex, endLocation + firstLaneDirection * shiftCurrent + FVector(0, 0, 500), ESplineCoordinateSpace::World);

				trafficSplineArray[i]->SetTangentAtSplinePoint(currentLoopIndex, startTangent, ESplineCoordinateSpace::World);
				trafficSplineArray[i]->SetTangentAtSplinePoint(nextLoopIndex, endTangent, ESplineCoordinateSpace::World);
			}
			else
			{
				trafficSplineArray[i]->SetLocationAtSplinePoint(currentLoopIndex, startLocation + secondLaneDirection * shiftCurrent + FVector(0, 0, 500), ESplineCoordinateSpace::World);
				trafficSplineArray[i]->SetLocationAtSplinePoint(nextLoopIndex, endLocation + secondLaneDirection * shiftCurrent + FVector(0, 0, 500), ESplineCoordinateSpace::World);

				trafficSplineArray[i]->SetTangentAtSplinePoint(currentLoopIndex, startTangent, ESplineCoordinateSpace::World);
				trafficSplineArray[i]->SetTangentAtSplinePoint(nextLoopIndex, endTangent, ESplineCoordinateSpace::World);
			}

		}
	}

}