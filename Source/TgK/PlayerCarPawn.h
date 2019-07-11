// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Classes/Components/SkeletalMeshComponent.h"
#include "Classes/Components/InputComponent.h"
#include "Classes/Components/AudioComponent.h"
#include "Runtime/Engine/Classes/Sound/SoundCue.h"
#include "GameFramework/SpringArmComponent.h"
#include "Classes/Components/BoxComponent.h"
#include "Classes/Components/SphereComponent.h"
#include "Engine/SkeletalMesh.h"
#include "WheeledVehicle.h"
#include "WheeledVehicleMovementComponent4W.h"
#include "PlayerWheelFront.h"
#include "PlayerWheelRear.h"
#include "Runtime/Engine/Classes/Curves/CurveFloat.h"
#include "GameFramework/Pawn.h"
#include "Engine.h"
#include "PlayerCarPawn.generated.h"

UCLASS()
class TGK_API APlayerCarPawn : public AWheeledVehicle
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APlayerCarPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Spring arm component for third person camera
	UPROPERTY(EditAnywhere)
	USpringArmComponent* thirdPersonSpringArm;

	// Third person camera component
	UPROPERTY(EditAnywhere)
	UCameraComponent* thirdPersonCamera;

	// Scene component for first person camera
	UPROPERTY(EditAnywhere)
	USceneComponent* firstPersonCameraMount;

	// First person camera component
	UPROPERTY(EditAnywhere)
	UCameraComponent* firstPersonCamera;

	// Animation instance reference
	UAnimInstance* animationInstance;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Car mesh
	USkeletalMeshComponent* CarMesh;
	// Car movement component
	UWheeledVehicleMovementComponent4W* VehicleMovement;

	UPROPERTY(EditAnywhere)
	class AGameManager* gameManager;

#pragma region Luna Components

	UPROPERTY(EditAnywhere)
	USceneComponent* seatForLuna;

	UPROPERTY(EditAnywhere)
	USceneComponent* castPointForLuna;

	UPROPERTY(EditAnywhere)
	USkeletalMeshComponent* luna;

	UPROPERTY(EditAnywhere)
	UAnimInstance* lunaAnimInstance;

	UPROPERTY(EditAnywhere)
	UBoxComponent* castPointForLunaOverlap;
	bool isCastPointOverlapped = false;
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* overlappedComp, AActor* otherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

#pragma endregion Luna Components

#pragma region Luna Animation

	FName IS_USING_ABILITY1_PROPERTY = FName("isUsingAbility1");
	FName IS_USING_ABILITY2_PROPERTY = FName("isUsingAbility2");
	FName IS_USING_ABILITY3_PROPERTY = FName("isUsingAbility3");
	FName IS_ANIMATION_OVER_PROPERTY = FName("isAbilityAnimationOver");
	FName IS_CANCELLING_ANIMATION_PROPERTY = FName("isCancellingAbility");

	void MoveLunaToCastPoint();
	void MoveLunaInside();

	void HandleAbility1Animation();
	void HandleAbility2Animation();
	void HandleAbility3Animation();

	void CancelAnimation();

	bool isWaitingForAnimationToEnd = false;
	bool CheckIfAnimationIsOver();

#pragma endregion Luna Animation

#pragma region Car Variables

	float currentHealth = 100.f;
	float maxHealth = 100.f;
	float autoHealRate = .028f;

	float distanceRemaining;

#pragma endregion Car Variables

#pragma region Car Components

	UPROPERTY(EditAnywhere)
	UAudioComponent* engineSound;

	UPROPERTY(EditAnywhere)
	USoundCue* startEngineSound;

	UFUNCTION()
	UAudioComponent* playSoundCue(USoundCue *sound);

	static const FName ENGINEAUDIORPM;
	static const FName ENGINEAUDIOVOLUME;
	static const FName LIMITER;
	static const FName IDLE;
	static const FName OFF_THROTTLE;

	UCurveFloat* torqueCurve;
	UCurveFloat* powerCurve;

#pragma endregion Car Components

#pragma region Particles
	
	// Warp
	UParticleSystem* warpArmSphereParticleSystemValid;
	UParticleSystem* warpArmSphereParticleSystemInvalid;
	UParticleSystem* warpTargetBoxParticleSystemValid;
	UParticleSystem* warpTargetBoxParticleSystemInvalid;

	UParticleSystem* warpExecuteParticleSystem;

	bool isPlayingValidLocationParticleSystem = true;
	UParticleSystemComponent* warpArmSphereParticleSystemComponent;
	UParticleSystemComponent* warpTargetBoxParticleSystemComponent;
	UParticleSystemComponent* warpExecuteParticleSystemComponent;

	void PlayWarpParticles();
	void DestroyWarpParticles();
	void AdjustWarpParticles(bool valid);
	void PlayWarpExecuteParticles();

	// Slow
	UParticleSystem* slowExecuteParticleSystem;

	void PlaySlowExecuteParticles();

	// Wallride
	UParticleSystem* wallrideArmSphereParticleSystemValid;
	UParticleSystem* wallrideArmSphereParticleSystemInvalid;
	UParticleSystem* wallrideTargetBoxParticleSystemValid;
	UParticleSystem* wallrideTargetBoxParticleSystemInvalid;
	UParticleSystem* wallrideActiveParticleSystem;

	UParticleSystem* wallrideExecuteParticleSystem;

	bool isPlayingValidWallParticleSystem = true;
	UParticleSystemComponent* wallrideArmSphereParticleSystemComponent;
	UParticleSystemComponent* wallrideTargetBoxParticleSystemComponent;
	UParticleSystemComponent* wallrideActiveParticleSystemComponent;

	void PlayWallrideParticles();
	void DestroyWallrideParticles();
	void AdjustWallrideParticles(bool valid);
	void PlayWallrideExecuteParticles();



#pragma endregion Particles

#pragma region Car Operation

	bool isPhysicsNormal = true;

	void MoveForward(float f);
	void MoveRight(float f);
	void Turn(float f);

	void HandBrakePressed();
	void HandBrakeReleased();

	void GearUp();
	void GearDown();

	void StartToFindUpright();
	bool isFindingUpright = false;

	void AddHealth(float f);
	void AutoHealHealth();

	float previousFrameRPM = 0;
	bool hasPlayedOffThrottleSound = false;

#pragma endregion Car Operation

#pragma region Camera Operation

	FRotator thirdPersonSpringArmDefaultRotation = FRotator(-10.f, 0.f, 0.f);
	FRotator thirdPersonSpringArmLevelStartRotation = FRotator(-10.f, -180.f, 0.f);

	void TurnCamera(float f);
	void SwitchCamera();
	bool canSwitchCamera = true;

	void ResetCamera();

	UPROPERTY(EditAnywhere)
	bool isThidPersonCameraActive = true;

	UPROPERTY(EditAnywhere)
	bool isFirstPersonCameraActive = false;

	UPROPERTY(EditAnywhere)
	float baseCameraTurnRate;

	UPROPERTY(EditAnywhere)
	float baseCameraPitchRate;

#pragma endregion Camera Operation

#pragma region Ability Operation

	float currentEnergy = 2500.f;
	float maxEnergy = 3000.f;

	// Choose ability
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int currentAbility = 1;
	void AbilityNext();
	void AbilityPrevious();
	void Ability(int choice);
	template<int index>
	void Ability()
	{
		Ability(index);
	}

	bool canChooseAbility = true;

	// Use ability
	bool canUseAbility = true;
	bool isUsingAbility = false;

	bool canExecuteAbility = false;
	bool isExecutingAbility = false;

	void AbilityActivate();
	void AbilityExecute();
	void AbilityCancel();

	bool isGlobalAbilityCoolingDown = false;
	float globalCooldownTimeRemaining = 80.f;

	float globalCooldownMax = 80.f;

	void AddEnergy(float f);

	void AutoRechargeEnergy();
	float autoRechargeRate = .5;

	void UpdateSelectedAbilityUI();
	void UpdateSelectedGearUI();

#pragma endregion Ability Operation

#pragma region Ability Components

	UPROPERTY(EditAnywhere)
	USpringArmComponent* warpTargetArm;

	UPROPERTY(EditAnywhere)
	USphereComponent* warpArmSphere;

	UPROPERTY(EditAnywhere)
	UBoxComponent* warpTargetBox;

	UPROPERTY(EditAnywhere)
	USpringArmComponent* wallrideTargetArm;

	UPROPERTY(EditAnywhere)
	USphereComponent* wallrideArmSphere;

	UPROPERTY(EditAnywhere)
	UBoxComponent* wallrideTargetBox;

#pragma endregion Ability Components

#pragma region Ability 1 Control

	const float energyRequiredWarp = 500.f;

	bool isChoosingWarpDestination = false;
	void WarpSelectModeActive();
	void CancelWarpMode();
	void DoWarp();

	float warpRotationAngle = 180;
	float warpRotateAtRate = .5;
	void WarpDestinationMoveRight(float f);

	bool foundGroundAbove = false;
	void PrepareGroundProbe(bool up);
	FHitResult GroundProbe(const FVector &Start, const FVector &End, bool up);

	bool isWarpCoolingDown = false;
	float warpCooldownTimeRemaining = 180.f;
	float warpCooldownMax = 180.f;

	void WarpDestinationCheckOverlap();

#pragma endregion Warp Ability Control

#pragma region Ability 2 Control
	
	float energyRequiredSlow = 1000.f;

	bool isChoosingSlowTime = false;
	void SlowSelectModeActive();
	void CancelSlowMode();
	void DoSlow();

	bool isSlowCoolingDown = false;
	float slowCooldownTimeRemaining = 600.f;
	float slowCooldownMax = 600.f;

	bool isSlowActive = false;
	float slowActiveTime = 0.f;
	float slowActiveMaxTime = 450.f;

	float amountToSlow = .1f;

	void SetSlowMovement();
	void SetNormalMovement();

	void SlowMovement(float f);

#pragma endregion Slow Ability Control

#pragma region Ability 3 Control

	const float energyRequiredWallRide = 750;

	bool isChoosingWallrideObject = false;
	void WallrideSelectModeActive();
	void CancelWallrideMode();
	void DoWallRide();

	float wallrideRotationAngle = 0;
	float wallrideRotateAtRate = 1;
	void WallrideDestinationMoveRight(float f);

	bool isWallrideCoolingDown = false;
	float wallrideCooldownTimeRemaining = 500.f;
	float wallrideCooldownMax = 500.f;

	class AAbility3Actor* ability3TargetActor;

	void PrepareWallProbe();
	FHitResult WallProbe(const FVector &Start, const FVector &End);

	void FindWallToRide();

	void WallrideDestinationCheckOverlap();

	bool isWallriding = false;

	void SetWallrideGravity();
	void SetNormalGravity();

	void WallrideMovement(float dTime);

	void AbortWallride();

	bool isResettingRoll = false;

#pragma endregion Wallride Ability Control

#pragma region Rescue Ability Control

	void RescueAbility();

#pragma endregion Rescue Ability Control

#pragma region Ability Sound

	UAudioComponent* abilityAudioComponent;
	UAudioComponent* impactAudioComponent;
	UAudioComponent* engineMiscAudioComponent;

	USoundCue* ability1LoopAudio;

	USoundCue* ability2LoopAudio;
	USoundCue* ability2ExitAudio;

	USoundCue* ability3LoopAudio;
	USoundCue* ability3WallrideAudio;
	USoundCue* ability3ExitAudio;

	USoundCue* powerSourceAudio;

	USoundCue* impactAudio;

	USoundCue* shiftDownAudio;
	USoundCue* offThrottleAudio;

	void PlayImpactSoundCue();
	void ChangeAndPlayAbilitySoundCue(USoundCue* cue);
	void StopAbilitySoundCue();
	void PlayShiftDownSoundCue();
	void PlayOffThrottleSoundCue();

#pragma endregion Ability Sound

#pragma region Other Interactions

	UFUNCTION()
	void OnHit(UPrimitiveComponent* comp, AActor* otherActor, UPrimitiveComponent* otherComp, FVector hitNormal, const FHitResult &hitResult);

	UFUNCTION()
	void OnPowerSourceOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnPowerSourceOverlapEnd(UPrimitiveComponent* overlappedComp, AActor* otherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(EditAnywhere)
	UBoxComponent* powerSourceOverlapVolume;

#pragma endregion Other Interactions

#pragma region Level EnterExit

	UAudioComponent* levelInteractionAudioComponent;
	void PlayLevelSound(USoundCue* cue);

	UPROPERTY(VisibleAnywhere)
	bool isEnteringLevel = false;
	UPROPERTY(VisibleAnywhere)
	bool isExitingLevel = false;
	UPROPERTY(VisibleAnywhere)
	bool hasLevelStarted = false;

	void StartEnterCountdown();
	void DoLevelExit();

	FVector cameraEndLevelPosition;
	FRotator cameraEndLevelRotation;

	float levelEnterTime = 300.f;
	float levelEnterMaxTime = 300.f;
	float levelExitTime = 300.f;
	float levelExitMaxTime = 300.f;

	USoundCue* enterLevelSoundCue;
	USoundCue* exitLevelSoundCue;

#pragma endregion Level EnterExit

#pragma region Level Control

	bool isWaitingToCheckRemainingDistance = true;
	float remainingDistanceCheckCooldownTimeRemaining = 10.f;
	float maxRemainingDistanceCheckCooldownTimeRemaining = 10.f;

	UFUNCTION(BlueprintCallable)
	float GetDistanceRemaining();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool isPaused = false;

	void Pause();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool isLevelOver = false;

	ULevel* currentLevel;
	UFUNCTION(BlueprintCallable)
	ULevel* GetCurrentLevelForRestart();

#pragma endregion Level Control

#pragma region UI

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> UIWidgetBase;
	class UTgKGameWidget* UIWidget;

 #pragma endregion UI Control

};