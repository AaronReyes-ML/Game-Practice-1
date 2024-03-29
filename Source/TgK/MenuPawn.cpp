// Fill out your copyright notice in the Description page of Project Settings.

#include "MenuPawn.h"


// Sets default values
AMenuPawn::AMenuPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	root = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(root);



}

// Called when the game starts or when spawned
void AMenuPawn::BeginPlay()
{
	Super::BeginPlay();
	controller = GetWorld()->GetFirstPlayerController();
	controller->bShowMouseCursor = true;
	controller->bEnableClickEvents = true;
	controller->bEnableMouseOverEvents = true;
}

// Called every frame
void AMenuPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMenuPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

