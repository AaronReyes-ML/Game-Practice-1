// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerWheelFront.h"
#include "ConstructorHelpers.h"
#include "TireConfig.h"


UPlayerWheelFront::UPlayerWheelFront()
{
	ShapeRadius = 33.5f;
	ShapeWidth = 15.f;

	bAffectedByHandbrake = false;
	SteerAngle = 45.f;

	//SuspensionForceOffset = -10.f;
	//SuspensionMaxRaise = 0.f;
	//SuspensionMaxDrop = 0.f;
	//SuspensionNaturalFrequency = 0.f;
	//SuspensionDampingRatio = 0.f;

	SuspensionNaturalFrequency = 10.f;

	static ConstructorHelpers::FObjectFinder<UTireConfig> configData(TEXT("/Game/TgR_Content/Vehicles/GTR/TireConfig/FrontWheelConfig.FrontWheelConfig"));
	TireConfig = configData.Object;
}

