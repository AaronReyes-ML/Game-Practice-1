// Fill out your copyright notice in the Description page of Project Settings.

#include "TrafficCarPawn.h"
#include "TrafficLane.h"
#include "ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine.h"
#include "TrafficManager.h"
#include "PlayerCarPawn.h"


// Sets default values
ATrafficCarPawn::ATrafficCarPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	trafficCarMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Traffic car mesh");
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> carMeshObject(TEXT("/Game/TgR_Content/Vehicles/Evo/evo"));
	trafficCarMesh->SetSkeletalMesh(carMeshObject.Object);
	SetRootComponent(trafficCarMesh);
	trafficCarMesh->SetSimulatePhysics(true);
	trafficCarMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	trafficCarMesh->SetCollisionObjectType(ECC_WorldDynamic);
	trafficCarMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
	trafficCarMesh->bGenerateOverlapEvents = true;
	trafficCarMesh->SetNotifyRigidBodyCollision(true);

	movementGuideSphere = CreateDefaultSubobject<USphereComponent>("Movement guide sphere");
	movementGuideSphere->SetWorldLocation(trafficCarMesh->GetComponentLocation());
	movementGuideSphere->SetHiddenInGame(true);
	movementGuideSphere->bGenerateOverlapEvents = false;

	playerCarOverlapVolume = CreateDefaultSubobject<UBoxComponent>("Player overlap volume");
	playerCarOverlapVolume->AttachToComponent(trafficCarMesh, FAttachmentTransformRules::SnapToTargetIncludingScale);
	playerCarOverlapVolume->SetBoxExtent(FVector(50, 1500, 1000));
	playerCarOverlapVolume->SetWorldLocation(trafficCarMesh->GetComponentLocation());
	playerCarOverlapVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	playerCarOverlapVolume->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Overlap);
	playerCarOverlapVolume->SetHiddenInGame(true);
	playerCarOverlapVolume->bGenerateOverlapEvents = true;
}

// Called when the game starts or when spawned
void ATrafficCarPawn::BeginPlay()
{
	Super::BeginPlay();
	trafficCarMesh->OnComponentHit.AddDynamic(this, &ATrafficCarPawn::OnHit);
	playerCarOverlapVolume->OnComponentBeginOverlap.AddDynamic(this, &ATrafficCarPawn::OnOverlapBegin);
}

// Called every frame
void ATrafficCarPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (trafficManager->isTrafficActive && !isRecovering)
	{
		movementGuideSphereAdditionalDistance += 10;
		totalDistanceTraveled = movementGuideSphereInitialDistanceOnSpline + movementGuideSphereAdditionalDistance;
		if (totalDistanceTraveled < trafficLaneSpine->GetSplineLength())
		{
			movementGuideSphere->SetWorldLocation(trafficLaneSpine->GetLocationAtDistanceAlongSpline(totalDistanceTraveled, ESplineCoordinateSpace::World));
		}
		else
		{
			trafficManager->DoCarDestroy(this);
		}

		FVector vectorToSphere = movementGuideSphere->GetComponentLocation() - trafficCarMesh->GetComponentLocation();
		vectorToSphere.Normalize();
		vectorToSphere = FVector(1.f, 1.f, 0.f) * vectorToSphere;
		FVector speedAdjustedMove = vectorToSphere * 1100 * DeltaTime;

		trafficCarMesh->AddWorldOffset(speedAdjustedMove, false, (FHitResult *)nullptr, ETeleportType::TeleportPhysics);
		trafficCarMesh->SetWorldRotation(UKismetMathLibrary::MakeRotFromZX(trafficCarMesh->GetUpVector(), vectorToSphere), false, (FHitResult *)nullptr, ETeleportType::TeleportPhysics);
	}

}

// Called to bind functionality to input
void ATrafficCarPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ATrafficCarPawn::OnHit(UPrimitiveComponent* comp, AActor* otherActor, UPrimitiveComponent* otherComp, FVector impactNormal, const FHitResult &hitResult)
{
	if (otherActor->GetClass() == APlayerCarPawn::StaticClass())
	{
		isRecovering = true;
	}
}

void ATrafficCarPawn::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//GEngine->AddOnScreenDebugMessage(1, 10, FColor::White, FString("OVERLAPPED")); FScreenMessageString* mCurrentHealth = GEngine->ScreenMessages.Find(1); mCurrentHealth->TextScale.X = mCurrentHealth->TextScale.Y = 1.0f;
	if (OtherActor->GetClass() == APlayerCarPawn::StaticClass())
	{
		trafficManager->CarHasBeenPassed(this);
		//GEngine->AddOnScreenDebugMessage(1, 10, FColor::White, FString("PASSED")); FScreenMessageString* mCurrentHealth = GEngine->ScreenMessages.Find(1); mCurrentHealth->TextScale.X = mCurrentHealth->TextScale.Y = 1.0f;
	}
}

