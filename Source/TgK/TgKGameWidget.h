// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TgKGameWidget.generated.h"

/**
 * 
 */
UCLASS()
class TGK_API UTgKGameWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateSpedometer(float speed);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateTac(float rpm);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateAbilityText(const FString &abilitText);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateGearText(const FString &gearText);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ShowPaused();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void HidePaused();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ShowLevelOver();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateProgressIndicator(float progressPercent, int distanceToGo);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateDistanceToGoText(int distanceToGo);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateHealthBar(float percentage);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateEnergyBar(float percentage);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateGlobalCooldownBar(float percentage);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateWarpCooldownBar(float percentage);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateSlowCooldownBar(float percentage);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateWallrideCooldownBar(float percentage);
};
