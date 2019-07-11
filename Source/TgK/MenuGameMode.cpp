// Fill out your copyright notice in the Description page of Project Settings.

#include "MenuGameMode.h"
#include "MenuPawn.h"
#include "ConstructorHelpers.h"


AMenuGameMode::AMenuGameMode()
{
	DefaultPawnClass = AMenuPawn::StaticClass();
}