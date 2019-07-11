// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Engine.h"
#include "PlayerCarPawn.h"
#include "Ability3Actor.generated.h"

UCLASS()
class TGK_API AAbility3Actor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAbility3Actor();

	UPROPERTY(EditAnywhere)
	APlayerCarPawn* playerRef;

	UPROPERTY(EditAnywhere)
	USceneComponent* root;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* visibleMesh;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* rideSurfaceMesh;

	UPROPERTY(EditAnywhere)
	UMaterial* targetedMaterial;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* defaultMaterial;

	bool isAlreadyOverlapped = false;
	bool hasChangedMaterial = false;
	void DoMaterialChange();
	void DoMaterialReset();

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* overlappedComp, AActor* otherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
