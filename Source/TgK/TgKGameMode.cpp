// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "TgKGameMode.h"
#include "TgKPawn.h"
#include "PlayerCarPawn.h"
#include "TgKHud.h"

ATgKGameMode::ATgKGameMode()
{
	DefaultPawnClass = APlayerCarPawn::StaticClass();
	HUDClass = ATgKHud::StaticClass();
}
