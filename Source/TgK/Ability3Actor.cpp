// Fill out your copyright notice in the Description page of Project Settings.

#include "Ability3Actor.h"
#include "ConstructorHelpers.h"


// Sets default values
AAbility3Actor::AAbility3Actor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	root = CreateDefaultSubobject<USceneComponent>("Root");

	visibleMesh = CreateDefaultSubobject<UStaticMeshComponent>("Visible Mesh");
	// Make WorldStatic
	visibleMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	// Ability 1 Trace - Ignore
	visibleMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Ignore);
	// Ability 3 Object - Ignore
	visibleMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Block);
	// Ability 3 Trace - Ignore
	visibleMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel3, ECollisionResponse::ECR_Ignore);
	visibleMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);

	rideSurfaceMesh = CreateDefaultSubobject<UStaticMeshComponent>("Ride surface mesh");
	// Make Ability3Object
	rideSurfaceMesh->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel2);
	// Ability3Object - Overlap
	rideSurfaceMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Overlap);
	// Ability3Trace - Block
	rideSurfaceMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel3, ECollisionResponse::ECR_Block);
	rideSurfaceMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	rideSurfaceMesh->SetVisibility(false);


	static ConstructorHelpers::FObjectFinder<UMaterial> tM(TEXT("/Game/TgR_Content/Meshes/test/Tunnel/targeted"));
	targetedMaterial = tM.Object;
}

// Called when the game starts or when spawned
void AAbility3Actor::BeginPlay()
{
	Super::BeginPlay();

	rideSurfaceMesh->OnComponentBeginOverlap.AddDynamic(this, &AAbility3Actor::OnOverlapBegin);
	rideSurfaceMesh->OnComponentEndOverlap.AddDynamic(this, &AAbility3Actor::OnOverlapEnd);

	defaultMaterial = visibleMesh->GetMaterial(0);

	if (GetWorld())
	{
		for (TActorIterator<APlayerCarPawn> playerIterator(GetWorld()); playerIterator; ++playerIterator)
		{
			APlayerCarPawn* foundPlayer = *playerIterator;
			if (foundPlayer != nullptr)
			{
				if (playerRef != foundPlayer)
				{
					playerRef = foundPlayer;
				}
			}
		}
	}
}

// Called every frame
void AAbility3Actor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAbility3Actor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (playerRef->isChoosingWallrideObject && !hasChangedMaterial)
	{
		visibleMesh->SetMaterial(0, targetedMaterial);
		hasChangedMaterial = true;
	}
	else
	{
		isAlreadyOverlapped = true;
	}
}

void AAbility3Actor::OnOverlapEnd(UPrimitiveComponent* overlappedComp, AActor* otherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	isAlreadyOverlapped = false;

	if (playerRef->isChoosingWallrideObject || hasChangedMaterial)
	{
		visibleMesh->SetMaterial(0, defaultMaterial);
		hasChangedMaterial = false;
	}
}

void AAbility3Actor::DoMaterialChange()
{
	visibleMesh->SetMaterial(0, targetedMaterial);
	hasChangedMaterial = true;
}

void AAbility3Actor::DoMaterialReset()
{
	visibleMesh->SetMaterial(0, defaultMaterial);
	hasChangedMaterial = false;
}