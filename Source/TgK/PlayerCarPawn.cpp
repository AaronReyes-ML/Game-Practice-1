// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCarPawn.h"
#include "ConstructorHelpers.h"
#include "Ability3Actor.h"
#include "PowerSource.h"
#include "TgKGameWidget.h"
#include "Blueprint/UserWidget.h"
#include "GameManager.h"
#include "Kismet/KismetMathLibrary.h"

const FName APlayerCarPawn::ENGINEAUDIORPM("RPM");
const FName APlayerCarPawn::ENGINEAUDIOVOLUME("Volume");
const FName APlayerCarPawn::LIMITER("LIMITER");
const FName APlayerCarPawn::IDLE("IDLE");
const FName APlayerCarPawn::OFF_THROTTLE("OFFTHROTTLE");

#pragma region Creation/Startup

// Sets default values
APlayerCarPawn::APlayerCarPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Automatically use this pawn as the player
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// Create carMesh object
	CarMesh = GetMesh();
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> objMesh(TEXT("/Game/TgR_Content/Vehicles/r34_GTR/r34_gtr"));
	CarMesh->SetSkeletalMesh(objMesh.Object);
	CarMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel5, ECollisionResponse::ECR_Ignore);
	CarMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel7, ECR_Overlap);

	// Create animation blueprint
	static ConstructorHelpers::FClassFinder<UAnimInstance> objAnimClass(TEXT("/Game/TgR_Content/Blueprints/Animation/r34_gtr_AnimBP"));
	CarMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	CarMesh->SetAnimInstanceClass(objAnimClass.Class);

	// Setup vehicle movement component
	VehicleMovement = CastChecked<UWheeledVehicleMovementComponent4W>(GetVehicleMovement());

	// Wheel setup
	//check(VehicleMovement->WheelSetups.Num() == 4);

	VehicleMovement->WheelSetups[0].WheelClass = UPlayerWheelFront::StaticClass();
	VehicleMovement->WheelSetups[0].BoneName = FName(TEXT("FrontLeftBone"));
	VehicleMovement->WheelSetups[0].AdditionalOffset = FVector(0.f, -12.f, 0.f);

	VehicleMovement->WheelSetups[1].WheelClass = UPlayerWheelFront::StaticClass();
	VehicleMovement->WheelSetups[1].BoneName = FName(TEXT("FrontRightBone"));
	VehicleMovement->WheelSetups[1].AdditionalOffset = FVector(0.f, 12.f, 0.f);

	VehicleMovement->WheelSetups[2].WheelClass = UPlayerWheelRear::StaticClass();
	VehicleMovement->WheelSetups[2].BoneName = FName(TEXT("RearLeftBone"));
	VehicleMovement->WheelSetups[2].AdditionalOffset = FVector(0.f, -12.f, 0.f);

	VehicleMovement->WheelSetups[3].WheelClass = UPlayerWheelRear::StaticClass();
	VehicleMovement->WheelSetups[3].BoneName = FName(TEXT("RearRightBone"));
	VehicleMovement->WheelSetups[3].AdditionalOffset = FVector(0.f, 12.f, 0.f);


	// Tire setup
	VehicleMovement->MinNormalizedTireLoad = 0.f;
	VehicleMovement->MinNormalizedTireLoadFiltered = 1.f;
	VehicleMovement->MaxNormalizedTireLoad = 5.f;
	VehicleMovement->MaxNormalizedTireLoadFiltered = 5.f;

	// Engine torque settings
	VehicleMovement->EngineSetup.MaxRPM = 10000.f;
	VehicleMovement->EngineSetup.TorqueCurve.GetRichCurve()->Reset();
	VehicleMovement->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(0.0f, 200.f);
	VehicleMovement->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(1000.f, 600.f);
	VehicleMovement->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(4500.f, 950.f);
	VehicleMovement->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(10000.f, 750.f);
	VehicleMovement->EngineSetup.DampingRateFullThrottle = 0.15f;
	VehicleMovement->EngineSetup.DampingRateZeroThrottleClutchEngaged = 2.f;
	VehicleMovement->EngineSetup.DampingRateZeroThrottleClutchDisengaged = 0.35f;

	// Steering settings
	VehicleMovement->SteeringCurve.GetRichCurve()->Reset();
	VehicleMovement->SteeringCurve.GetRichCurve()->AddKey(0.f, 1.f);
	VehicleMovement->SteeringCurve.GetRichCurve()->AddKey(40.f, 0.7f);
	VehicleMovement->SteeringCurve.GetRichCurve()->AddKey(120.0f, 0.6f);

	// Transmition settings
	VehicleMovement->DifferentialSetup.DifferentialType = EVehicleDifferential4W::LimitedSlip_4W;
	VehicleMovement->TransmissionSetup.bUseGearAutoBox = false;
	VehicleMovement->TransmissionSetup.GearSwitchTime = 0.3f;
	VehicleMovement->TransmissionSetup.GearAutoBoxLatency = 1.f;

	CarMesh->SetLinearDamping(.01f);

	// Third person camera spring arm setup
	thirdPersonSpringArm = CreateDefaultSubobject<USpringArmComponent>("Third person arm");
	thirdPersonSpringArm->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	thirdPersonSpringArm->SetRelativeLocation(FVector(-70.f, 0.f, 110.f));
	thirdPersonSpringArm->SetRelativeRotation(FRotator(-10.f, 0.f, 0.f));
	thirdPersonSpringArm->TargetArmLength = 500.f;
	thirdPersonSpringArm->bEnableCameraLag = false;
	thirdPersonSpringArm->bEnableCameraRotationLag = false;
	thirdPersonSpringArm->bInheritPitch = true;
	thirdPersonSpringArm->bInheritYaw = true;
	thirdPersonSpringArm->bInheritRoll = true;

	// Third person camera component
	thirdPersonCamera = CreateDefaultSubobject<UCameraComponent>("Third person camera");
	thirdPersonCamera->SetupAttachment(thirdPersonSpringArm, USpringArmComponent::SocketName);
	thirdPersonCamera->SetFieldOfView(100.f);
	thirdPersonCamera->bConstrainAspectRatio = true;
	thirdPersonCamera->bUsePawnControlRotation = false;
	thirdPersonCamera->bAutoActivate = true;

	// First person camera holder setup
	firstPersonCameraMount = CreateDefaultSubobject<USceneComponent>("First person mount");
	firstPersonCameraMount->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	firstPersonCameraMount->SetRelativeLocation(FVector(-5.f, 35.f, 115.f));

	// First person camera setup
	firstPersonCamera = CreateDefaultSubobject<UCameraComponent>("First person camera");
	firstPersonCamera->SetupAttachment(firstPersonCameraMount);
	firstPersonCamera->SetFieldOfView(100.f);
	firstPersonCamera->bConstrainAspectRatio = true;
	firstPersonCamera->bUsePawnControlRotation = false;
	firstPersonCamera->bAutoActivate = false;

	engineSound = CreateDefaultSubobject<UAudioComponent>("Engine sound");
	static ConstructorHelpers::FObjectFinder<USoundCue> SC(TEXT("/Game/TgR_Content/Audo/VehicleAudio/new_GTR/new_EngineSoundCue"));
	engineSound->SetSound(SC.Object);
	engineSound->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);

	warpTargetArm = CreateDefaultSubobject<USpringArmComponent>("Warp target arm");
	warpTargetArm->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	warpTargetArm->TargetArmLength = 1000;
	warpTargetArm->bDoCollisionTest = false; // Don't 
	warpTargetArm->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	warpTargetArm->SetHiddenInGame(true);

	warpTargetBox = CreateDefaultSubobject<UBoxComponent>("Warp target box");
	warpTargetBox->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	warpTargetBox->SetBoxExtent(FVector(245.f, 100.f, 70.f));
	warpTargetBox->SetRelativeLocation(FVector(0.f, 0.f, 70.f));
	warpTargetBox->ShapeColor = FColor::Blue;
	warpTargetBox->SetHiddenInGame(true);
	warpTargetBox->bGenerateOverlapEvents = false;
	warpTargetBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel5, ECollisionResponse::ECR_Ignore);

	warpArmSphere = CreateDefaultSubobject<USphereComponent>("Warp arm sphere");
	warpArmSphere->SetupAttachment(warpTargetArm, USpringArmComponent::SocketName);
	warpArmSphere->bGenerateOverlapEvents = false;
	warpArmSphere->SetHiddenInGame(true);

	wallrideTargetArm = CreateDefaultSubobject<USpringArmComponent>("Wallride target arm");
	wallrideTargetArm->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	wallrideTargetArm->TargetArmLength = 1000;
	wallrideTargetArm->SetRelativeRotation(FRotator(0.f, 145.f, 0.f));
	wallrideTargetArm->SetRelativeLocation(FVector(0.f, 0.f, 70.f));
	wallrideTargetArm->ProbeChannel = ECollisionChannel::ECC_GameTraceChannel2;
	wallrideTargetArm->SetHiddenInGame(true);

	wallrideArmSphere = CreateDefaultSubobject<USphereComponent>("Wallride target sphere");
	wallrideArmSphere->SetupAttachment(wallrideTargetArm, USpringArmComponent::SocketName);
	wallrideArmSphere->SetSphereRadius(100.f);
	wallrideArmSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	wallrideArmSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Overlap);
	wallrideArmSphere->bGenerateOverlapEvents = true;
	wallrideArmSphere->SetHiddenInGame(true);

	wallrideTargetBox = CreateDefaultSubobject<UBoxComponent>("Wallride target box");
	wallrideTargetBox->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	wallrideTargetBox->SetBoxExtent(FVector(245.f, 100.f, 70.f));
	wallrideTargetBox->SetRelativeLocation(FVector(0.f, 0.f, 70.f));
	wallrideTargetBox->ShapeColor = FColor::Blue;
	wallrideTargetBox->SetHiddenInGame(true);
	wallrideTargetBox->bGenerateOverlapEvents = false;
	wallrideTargetBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel5, ECollisionResponse::ECR_Ignore);

	seatForLuna = CreateDefaultSubobject<USceneComponent>("Luna's seat");
	seatForLuna->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	seatForLuna->SetRelativeLocation(FVector(3.5f, -40.f, 4.5f));
	seatForLuna->SetRelativeRotation(FRotator(7.f, 0.f, 0.f));

	castPointForLuna = CreateDefaultSubobject<USceneComponent>("Luna's cast point");
	castPointForLuna->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	castPointForLuna->SetRelativeLocation(FVector(-60.4f, -20.5f, 131.f));
	castPointForLuna->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));

	castPointForLunaOverlap = CreateDefaultSubobject<UBoxComponent>("Luna's cast point overlap");
	castPointForLunaOverlap->AttachToComponent(castPointForLuna, FAttachmentTransformRules::SnapToTargetIncludingScale);
	castPointForLunaOverlap->SetRelativeLocation(FVector(0.f, 0.f, 75.f));
	castPointForLunaOverlap->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	castPointForLunaOverlap->SetBoxExtent(FVector(48.f, 43.5f, 66.f));
	castPointForLunaOverlap->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel5);
	castPointForLunaOverlap->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	castPointForLunaOverlap->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Overlap);
	castPointForLunaOverlap->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	castPointForLunaOverlap->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel5, ECollisionResponse::ECR_Overlap);

	luna = CreateDefaultSubobject<USkeletalMeshComponent>("Luna");
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> lunaMeshObject(TEXT("/Game/TgR_Content/Characters/Luna/Base/luna"));
	luna->SetSkeletalMesh(lunaMeshObject.Object);
	luna->AttachToComponent(seatForLuna, FAttachmentTransformRules::SnapToTargetIncludingScale);
	luna->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	luna->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	luna->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	static ConstructorHelpers::FClassFinder<UObject> lunaAnimBPClass(TEXT("/Game/TgR_Content/Blueprints/Animation/luna_AnimBP"));
	luna->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	luna->SetAnimInstanceClass(lunaAnimBPClass.Class);

	warpArmSphereParticleSystemValid = CreateDefaultSubobject<UParticleSystem>("Warp arm sphere particle system valid");
	static ConstructorHelpers::FObjectFinder<UParticleSystem> particleSystemObject1(TEXT("/Game/InfinityBladeEffects/Effects/FX_Ability/Summon/Valid"));
	warpArmSphereParticleSystemValid = particleSystemObject1.Object;

	warpArmSphereParticleSystemInvalid = CreateDefaultSubobject<UParticleSystem>("Warp arm sphere particle system invalid");
	static ConstructorHelpers::FObjectFinder<UParticleSystem> particleSystemObject2(TEXT("/Game/InfinityBladeEffects/Effects/FX_Ability/Summon/Invalid"));
	warpArmSphereParticleSystemInvalid = particleSystemObject2.Object;

	warpTargetBoxParticleSystemValid = CreateDefaultSubobject<UParticleSystem>("Warp target box particle system valid");
	static ConstructorHelpers::FObjectFinder<UParticleSystem> particleSystemObject3(TEXT("/Game/InfinityBladeEffects/Effects/FX_Ability/Defense/P_Shield_Sphere_Buff1"));
	warpTargetBoxParticleSystemValid = particleSystemObject3.Object;

	warpTargetBoxParticleSystemInvalid = CreateDefaultSubobject<UParticleSystem>("Warp target box particle system invalid");
	static ConstructorHelpers::FObjectFinder<UParticleSystem> particleSystemObject4(TEXT("/Game/InfinityBladeEffects/Effects/FX_Ability/Defense/P_Shield_Sphere_Buff2"));
	warpTargetBoxParticleSystemInvalid = particleSystemObject4.Object;

	warpExecuteParticleSystem = CreateDefaultSubobject<UParticleSystem>("Warp execute particle system");
	static ConstructorHelpers::FObjectFinder<UParticleSystem> particleSystemObject5(TEXT("/Game/InfinityBladeEffects/Effects/FX_Skill_Aura/P_Aura_Ice_Upheaval_01"));
	warpExecuteParticleSystem = particleSystemObject5.Object;

	slowExecuteParticleSystem = CreateDefaultSubobject<UParticleSystem>("Slow execute particle system");
	static ConstructorHelpers::FObjectFinder<UParticleSystem> particleSystemObject6(TEXT("/Game/InfinityBladeEffects/Effects/FX_Skill_Aura/P_AuraCircle_Default_StartUp_01"));
	slowExecuteParticleSystem = particleSystemObject6.Object;

	wallrideArmSphereParticleSystemValid = CreateDefaultSubobject<UParticleSystem>("Wallride target sphere particle system valid");
	static ConstructorHelpers::FObjectFinder<UParticleSystem> particleSystemObject7(TEXT("/Game/InfinityBladeEffects/Effects/FX_Ability/Summon/Valid"));
	wallrideArmSphereParticleSystemValid = particleSystemObject7.Object;

	wallrideArmSphereParticleSystemInvalid = CreateDefaultSubobject<UParticleSystem>("Wallride target sphere particle system invalid");
	static ConstructorHelpers::FObjectFinder<UParticleSystem> particleSystemObject8(TEXT("/Game/InfinityBladeEffects/Effects/FX_Ability/Summon/Invalid"));
	wallrideArmSphereParticleSystemInvalid = particleSystemObject8.Object;

	wallrideTargetBoxParticleSystemValid = CreateDefaultSubobject<UParticleSystem>("Wallride target box particle system valid");
	static ConstructorHelpers::FObjectFinder<UParticleSystem> particleSystemObject9(TEXT("/Game/InfinityBladeEffects/Effects/FX_Ability/Defense/P_Shield_Sphere_Buff1"));
	wallrideTargetBoxParticleSystemValid = particleSystemObject9.Object;

	wallrideTargetBoxParticleSystemInvalid = CreateDefaultSubobject<UParticleSystem>("Wallride target box particle system invalid");
	static ConstructorHelpers::FObjectFinder<UParticleSystem> particleSystemObject10(TEXT("/Game/InfinityBladeEffects/Effects/FX_Ability/Defense/P_Shield_Sphere_Buff2"));
	wallrideTargetBoxParticleSystemInvalid = particleSystemObject10.Object;

	wallrideExecuteParticleSystem = CreateDefaultSubobject<UParticleSystem>("Wallride execute particle system");
	static ConstructorHelpers::FObjectFinder<UParticleSystem> particleSystemObject11(TEXT("/Game/InfinityBladeEffects/Effects/FX_Skill_TeleCharge/P_Skill_Telecharge_Shock_Impact_02"));
	wallrideExecuteParticleSystem = particleSystemObject11.Object;

	wallrideActiveParticleSystem = CreateDefaultSubobject<UParticleSystem>("Wallride active particle system");
	static ConstructorHelpers::FObjectFinder<UParticleSystem> particleSystemObject12(TEXT("/Game/InfinityBladeEffects/Effects/FX_Skill_Aura/P_AuraCircle_Temp"));
	wallrideActiveParticleSystem = particleSystemObject12.Object;

	abilityAudioComponent = CreateDefaultSubobject<UAudioComponent>("Ability audio component");
	abilityAudioComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);

	levelInteractionAudioComponent = CreateDefaultSubobject<UAudioComponent>("Power source audio component");
	levelInteractionAudioComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);

	impactAudioComponent = CreateDefaultSubobject<UAudioComponent>("Impact audio component");
	impactAudioComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);

	powerSourceAudio = CreateDefaultSubobject<USoundCue>("Power source audio");
	static ConstructorHelpers::FObjectFinder<USoundCue> soundCue0(TEXT("/Game/TgR_Content/Audo/AbilityAudio/PowerSource"));
	powerSourceAudio = soundCue0.Object;

	impactAudio = CreateDefaultSubobject<USoundCue>("Impact audio");
	static ConstructorHelpers::FObjectFinder<USoundCue> soundCue01(TEXT("/Game/TgR_Content/Audo/AbilityAudio/Impact"));
	impactAudio = soundCue01.Object;

	ability1LoopAudio = CreateDefaultSubobject<USoundCue>("Ability 1 loop audio");
	static ConstructorHelpers::FObjectFinder<USoundCue> soundCue1(TEXT("/Game/TgR_Content/Audo/AbilityAudio/Ability1Loop"));
	ability1LoopAudio = soundCue1.Object;

	ability2LoopAudio = CreateDefaultSubobject<USoundCue>("Ability 2 loop audio");
	static ConstructorHelpers::FObjectFinder<USoundCue> soundCue2(TEXT("/Game/TgR_Content/Audo/AbilityAudio/Ability2Loop"));
	ability2LoopAudio = soundCue2.Object;

	ability2ExitAudio = CreateDefaultSubobject<USoundCue>("Ability 2 exit audio");
	static ConstructorHelpers::FObjectFinder<USoundCue> soundCue3(TEXT("/Game/TgR_Content/Audo/AbilityAudio/Ability2Execute"));
	ability2ExitAudio = soundCue3.Object;

	ability3LoopAudio = CreateDefaultSubobject<USoundCue>("Ability 3 loop audio");
	static ConstructorHelpers::FObjectFinder<USoundCue> soundCue4(TEXT("/Game/TgR_Content/Audo/AbilityAudio/Ability3Loop"));
	ability3LoopAudio = soundCue4.Object;

	ability3WallrideAudio = CreateDefaultSubobject<USoundCue>("Ability 3 wallride audio");
	static ConstructorHelpers::FObjectFinder<USoundCue> soundCue5(TEXT("/Game/TgR_Content/Audo/AbilityAudio/Ability3WallrideLoop"));
	ability3WallrideAudio = soundCue5.Object;

	ability3ExitAudio = CreateDefaultSubobject<USoundCue>("Ability 3 exit audio");
	static ConstructorHelpers::FObjectFinder<USoundCue> soundCue6(TEXT("/Game/TgR_Content/Audo/AbilityAudio/Ability3Execute"));
	ability3ExitAudio = soundCue6.Object;

	enterLevelSoundCue = CreateDefaultSubobject<USoundCue>("Enter level audio");
	static ConstructorHelpers::FObjectFinder<USoundCue> soundCue7(TEXT("/Game/TgR_Content/Audo/VehicleAudio/LFA/LFA_eginestart"));
	enterLevelSoundCue = soundCue7.Object;

	exitLevelSoundCue = CreateDefaultSubobject<USoundCue>("Exit level audio");
	static ConstructorHelpers::FObjectFinder<USoundCue> soundCue8(TEXT("/Game/TgR_Content/Audo/AbilityAudio/Ability3Execute"));
	exitLevelSoundCue = soundCue8.Object;

	shiftDownAudio = CreateDefaultSubobject<USoundCue>("Shift down audio");
	static ConstructorHelpers::FObjectFinder<USoundCue> soundCue9(TEXT("/Game/TgR_Content/Audo/VehicleAudio/new_GTR/new_ShiftDownSondCue"));
	shiftDownAudio = soundCue9.Object;

	offThrottleAudio = CreateDefaultSubobject<USoundCue>("Off throttle audio");
	static ConstructorHelpers::FObjectFinder<USoundCue> soundCue10(TEXT("/Game/TgR_Content/Audo/VehicleAudio/new_GTR/new_OffThrottleSoundCue"));
	offThrottleAudio = soundCue10.Object;

	engineMiscAudioComponent = CreateDefaultSubobject<UAudioComponent>("Misc engine sounds audio component");
	engineMiscAudioComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);

	powerSourceOverlapVolume = CreateDefaultSubobject<UBoxComponent>("Power source overlap volume");
	powerSourceOverlapVolume->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	powerSourceOverlapVolume->SetBoxExtent(FVector(245.f, 100.f, 70.f));
	powerSourceOverlapVolume->SetRelativeLocation(FVector(0.f, 0.f, 70.f));
	powerSourceOverlapVolume->SetCollisionObjectType(ECC_GameTraceChannel6);
	powerSourceOverlapVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	powerSourceOverlapVolume->SetCollisionResponseToChannel(ECC_GameTraceChannel6, ECR_Overlap);
	powerSourceOverlapVolume->bGenerateOverlapEvents = true;
	powerSourceOverlapVolume->SetHiddenInGame(true);

	static ConstructorHelpers::FClassFinder<UTgKGameWidget> uiClass(TEXT("/Game/TgR_Content/Blueprints/UI/TgKGameWidgetBP"));
	UIWidgetBase = uiClass.Class;

	// Add tag to actor
	this->Tags.Add("PlayerCar");
}

// Called when the game starts or when spawned
void APlayerCarPawn::BeginPlay()
{
	Super::BeginPlay();

	currentLevel = GetWorld()->GetCurrentLevel();

	thirdPersonSpringArm->SetRelativeRotation(thirdPersonSpringArmLevelStartRotation);

	castPointForLunaOverlap->OnComponentBeginOverlap.AddDynamic(this, &APlayerCarPawn::OnOverlapBegin);
	castPointForLunaOverlap->OnComponentEndOverlap.AddDynamic(this, &APlayerCarPawn::OnOverlapEnd);

	powerSourceOverlapVolume->OnComponentBeginOverlap.AddDynamic(this, &APlayerCarPawn::OnPowerSourceOverlapBegin);

	UIWidget = Cast<UTgKGameWidget>(CreateWidget<UUserWidget>(GetWorld(), UIWidgetBase));
	if (UIWidget)
	{
		UIWidget->AddToViewport();
	}

	CarMesh->OnComponentHit.AddDynamic(this, &APlayerCarPawn::OnHit);

	engineSound->Stop();

	gameManager->PlayPreLevelSoundCue();

	distanceRemaining = gameManager->GetPlayerDistanceFromFinish();

	UIWidget->UpdateDistanceToGoText((int)floorf(distanceRemaining));
	gameManager->GetDistanceTraveled();
	UIWidget->UpdateProgressIndicator(gameManager->GetPercentDone(), (int)floorf(gameManager->GetPlayerDistanceFromFinish()));
	UIWidget->UpdateAbilityText(FString("WARP"));

	if (CarMesh)
	{
		animationInstance = Cast<UAnimInstance>(CarMesh->GetAnimInstance());
	}
	if (luna)
	{
		lunaAnimInstance = Cast<UAnimInstance>(luna->GetAnimInstance());
	}

}

// Called to bind functionality to input
void APlayerCarPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCarPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCarPawn::MoveRight);
	PlayerInputComponent->BindAxis("MouseRight", this, &APlayerCarPawn::WarpDestinationMoveRight);
	PlayerInputComponent->BindAxis("MouseRight", this, &APlayerCarPawn::WallrideDestinationMoveRight);
	PlayerInputComponent->BindAxis("MouseRight", this, &APlayerCarPawn::TurnCamera);

	PlayerInputComponent->BindAxis("TurnRight", this, &APlayerCarPawn::Turn);

	PlayerInputComponent->BindAction("Handbrake", IE_Pressed, this, &APlayerCarPawn::HandBrakePressed);
	PlayerInputComponent->BindAction("Handbrake", IE_Released, this, &APlayerCarPawn::HandBrakeReleased);
	PlayerInputComponent->BindAction("SwitchCamera", IE_Pressed, this, &APlayerCarPawn::SwitchCamera);
	PlayerInputComponent->BindAction("ResetCamera", IE_Pressed, this, &APlayerCarPawn::ResetCamera);

	PlayerInputComponent->BindAction("AbilityNext", IE_Pressed, this, &APlayerCarPawn::AbilityNext);
	PlayerInputComponent->BindAction("AbilityPrevious", IE_Pressed, this, &APlayerCarPawn::AbilityPrevious);
	PlayerInputComponent->BindAction("Ability1", IE_Pressed, this, &APlayerCarPawn::Ability<1>);
	PlayerInputComponent->BindAction("Ability2", IE_Pressed, this, &APlayerCarPawn::Ability<2>);
	PlayerInputComponent->BindAction("Ability3", IE_Pressed, this, &APlayerCarPawn::Ability<3>);
	PlayerInputComponent->BindAction("Ability4", IE_Pressed, this, &APlayerCarPawn::Ability<4>);
	PlayerInputComponent->BindAction("AbilityRescue", IE_Pressed, this, &APlayerCarPawn::RescueAbility);

	PlayerInputComponent->BindAction("AbilityActivate", IE_Pressed, this, &APlayerCarPawn::AbilityActivate);
	PlayerInputComponent->BindAction("AbilityActivate", IE_Released, this, &APlayerCarPawn::AbilityExecute);
	PlayerInputComponent->BindAction("AbilityCancel", IE_Pressed, this, &APlayerCarPawn::AbilityCancel);

	PlayerInputComponent->BindAction("GearUp", IE_Pressed, this, &APlayerCarPawn::GearUp);
	PlayerInputComponent->BindAction("GearDown", IE_Pressed, this, &APlayerCarPawn::GearDown);

	PlayerInputComponent->BindAction("StartLevel", IE_Pressed, this, &APlayerCarPawn::StartEnterCountdown);
	PlayerInputComponent->BindAction("Pause", IE_Pressed, this, &APlayerCarPawn::Pause);
}

#pragma endregion Creation/Startup

#pragma region Tick

// Called every frame
void APlayerCarPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (hasLevelStarted && !isPaused)
	{
		float rpmDelta = VehicleMovement->GetEngineRotationSpeed() - previousFrameRPM;
		if (rpmDelta < -100 && !hasPlayedOffThrottleSound && VehicleMovement->GetEngineRotationSpeed() > 6000)
		{
			PlayOffThrottleSoundCue();
			hasPlayedOffThrottleSound = true;
		}
		else if (rpmDelta > 0 && hasPlayedOffThrottleSound)
		{
			hasPlayedOffThrottleSound = false;
		}

		previousFrameRPM = VehicleMovement->GetEngineRotationSpeed();

		// Tick UI Updates
		UIWidget->UpdateSpedometer(FMath::Abs(GetVehicleMovement()->GetForwardSpeed()));
		UIWidget->UpdateTac(VehicleMovement->GetEngineRotationSpeed());

		AutoHealHealth();
		AutoRechargeEnergy();

		// Manage sound
		if (engineSound->bIsPaused)
		{
			engineSound->SetPaused(false);
		}
		engineSound->SetFloatParameter(ENGINEAUDIORPM, VehicleMovement->GetEngineRotationSpeed() / VehicleMovement->GetEngineMaxRotationSpeed());
		engineSound->SetFloatParameter(OFF_THROTTLE, FMath::Clamp(GetInputAxisValue("MoveForward"), 0.f, 1.f));
		engineSound->SetFloatParameter(LIMITER, VehicleMovement->GetEngineRotationSpeed());

		// Manage animation
		if (isWaitingForAnimationToEnd)
		{
			if (CheckIfAnimationIsOver())
			{
				isWaitingForAnimationToEnd = false;
				MoveLunaInside();
			}
		}

		// Manage cooldowns
		if (isGlobalAbilityCoolingDown)
		{
			if (globalCooldownTimeRemaining > 0)
			{
				globalCooldownTimeRemaining -= 1;
			}
			else
			{
				isGlobalAbilityCoolingDown = false;
			}
			UIWidget->UpdateGlobalCooldownBar(globalCooldownTimeRemaining / globalCooldownMax);
		}

		if (isWarpCoolingDown)
		{
			if (warpCooldownTimeRemaining > 0)
			{
				warpCooldownTimeRemaining -= 1;
			}
			else
			{
				isWarpCoolingDown = false;
			}
			UIWidget->UpdateWarpCooldownBar(warpCooldownTimeRemaining / warpCooldownMax);
		}

		if (isSlowCoolingDown)
		{
			if (slowCooldownTimeRemaining > 0)
			{
				slowCooldownTimeRemaining -= 1;
			}
			else
			{
				isSlowCoolingDown = false;
			}
			UIWidget->UpdateSlowCooldownBar(slowCooldownTimeRemaining / slowCooldownMax);
		}

		if (isWallrideCoolingDown)
		{
			if (wallrideCooldownTimeRemaining > 0)
			{
				wallrideCooldownTimeRemaining -= 1;
			}
			else
			{
				isWallrideCoolingDown = false;
			}
			UIWidget->UpdateWallrideCooldownBar(wallrideCooldownTimeRemaining / wallrideCooldownMax);
		}

		// Warp Ability
		// Only check for ground if we are currently choosing a destination
		if (isChoosingWarpDestination)
		{
			// Do GroundProbe up first
			PrepareGroundProbe(true);
			// Search down only if the ground was not found during upward search
			if (!foundGroundAbove)
			{
				// Do GroundProbe down
				PrepareGroundProbe(false);
			}
			// Only check overlap if we have a valid destination location
			if (canExecuteAbility)
			{
				// Check overlappingActors
				WarpDestinationCheckOverlap();
			}
			else // Failed to find a location
			{
				// Check to make sure we are not already playing invalid particles
				if (isPlayingValidLocationParticleSystem)
				{
					AdjustWarpParticles(false);
				}
			}
		}

		// Slow Ability
		// Only reduce slow time if slow is active
		if (isSlowActive)
		{
			if (slowActiveTime < slowActiveMaxTime)
			{
				slowActiveTime += 1;
			}
			else
			{
				CancelSlowMode();
			}
		}

		// Wallride Ability
		// Only check objects if we are currently choosing a destination
		if (isChoosingWallrideObject)
		{
			FindWallToRide();
			if (canExecuteAbility)
			{
				WallrideDestinationCheckOverlap();
			}
			else
			{
				if (isPlayingValidWallParticleSystem)
				{
					AdjustWallrideParticles(false);
				}
			}
		}

		// Non-Physics Based Movement
		if (!isPhysicsNormal)
		{
			// Wallride
			if (isWallriding)
			{
				WallrideMovement(DeltaTime);
			}
			else if (isSlowActive)
			{
				SlowMovement(DeltaTime);
			}
		}

		// Resets roll
		if (isResettingRoll)
		{
			if (CarMesh->GetComponentRotation().Roll > 4.f || CarMesh->GetComponentRotation().Roll < -4)
			{
				CarMesh->SetWorldRotation
				(FMath::RInterpConstantTo(CarMesh->GetComponentRotation(), FRotator(CarMesh->GetComponentRotation().Pitch, CarMesh->GetComponentRotation().Yaw, 0.f),
					DeltaTime, 150.f), false, (FHitResult *)nullptr, ETeleportType::TeleportPhysics);
			}
			else
			{
				isResettingRoll = false;
			}
		}

		if (isFindingUpright)
		{
			if (CarMesh->GetComponentRotation().Roll > 1.f || CarMesh->GetComponentRotation().Roll < -1)
			{
				CarMesh->SetWorldRotation
				(FMath::RInterpConstantTo(CarMesh->GetComponentRotation(), FRotator(CarMesh->GetComponentRotation().Pitch, CarMesh->GetComponentRotation().Yaw, 0.f),
					DeltaTime, 250.f), false, (FHitResult *)nullptr, ETeleportType::TeleportPhysics);
			}
			else
			{
				isFindingUpright = false;
			}
		}

		
		if (isWaitingToCheckRemainingDistance)
		{
			if (remainingDistanceCheckCooldownTimeRemaining > 0)
			{
				remainingDistanceCheckCooldownTimeRemaining -= 1;
			}
			else
			{
				gameManager->GetDistanceTraveled();
				UIWidget->UpdateProgressIndicator(gameManager->GetPercentDone(), (int)floorf(gameManager->GetPlayerDistanceFromFinish()));
				isWaitingToCheckRemainingDistance = true;
				remainingDistanceCheckCooldownTimeRemaining = maxRemainingDistanceCheckCooldownTimeRemaining;
			}
		}
		
    }
	else if (isEnteringLevel)
	{
		if (levelEnterTime > 0)
		{
			levelEnterTime -= 1;
			if (levelEnterTime <= 180)
			{
				thirdPersonSpringArm->SetRelativeRotation(UKismetMathLibrary::RInterpTo_Constant(thirdPersonSpringArm->RelativeRotation, thirdPersonSpringArmDefaultRotation, DeltaTime, 180.f));
			}
			VehicleMovement->SetThrottleInput(.75);
		}
		else
		{
			isEnteringLevel = false;
			hasLevelStarted = true;
		}
		engineSound->SetFloatParameter(ENGINEAUDIORPM, VehicleMovement->GetEngineRotationSpeed() / VehicleMovement->GetEngineMaxRotationSpeed());
	}
	else if (isExitingLevel)
	{
		VehicleMovement->SetThrottleInput(1.f);
		thirdPersonCamera->SetWorldLocation(cameraEndLevelPosition);
		thirdPersonCamera->SetWorldRotation(cameraEndLevelRotation);
	}
	else
	{
		if (engineSound->IsPlaying())
		{
			engineSound->SetPaused(true);
		}
	}

	//GEngine->AddOnScreenDebugMessage(1, 10, FColor::White, "Current Health                      : " + FString::SanitizeFloat(currentHealth) + "/" + FString::SanitizeFloat(maxHealth)); FScreenMessageString* mCurrentHealth = GEngine->ScreenMessages.Find(1); mCurrentHealth->TextScale.X = mCurrentHealth->TextScale.Y = 1.0f;
	//GEngine->AddOnScreenDebugMessage(2, 10, FColor::White, "Current Energy                        : " + FString::SanitizeFloat(currentEnergy) + "/" + FString::SanitizeFloat(maxEnergy)); FScreenMessageString* mCurrentEnergy = GEngine->ScreenMessages.Find(2); mCurrentEnergy->TextScale.X = mCurrentEnergy->TextScale.Y = 1.0f;
	//GEngine->AddOnScreenDebugMessage(1, 10, FColor::White, "Is Physics Normal                   : " + FString::Printf(TEXT("%s"), isPhysicsNormal ? TEXT("yes") : TEXT("no"))); FScreenMessageString* mIsPhysicsNormal = GEngine->ScreenMessages.Find(1); mIsPhysicsNormal->TextScale.X = mIsPhysicsNormal->TextScale.Y = 1.0f;
	//GEngine->AddOnScreenDebugMessage(2, 10, FColor::White, "Is Finding Upright                  : " + FString::Printf(TEXT("%s"), isResettingRoll ? TEXT("yes") : TEXT("no"))); FScreenMessageString* mIsFindingUpright = GEngine->ScreenMessages.Find(2); mIsFindingUpright->TextScale.X = mIsFindingUpright->TextScale.Y = 1.0f;
	//GEngine->AddOnScreenDebugMessage(3, 10, FColor::White, "Can Switch Camera                 : " + FString::Printf(TEXT("%s"), canSwitchCamera ? TEXT("yes") : TEXT("no"))); FScreenMessageString* mCamera = GEngine->ScreenMessages.Find(3); 	mCamera->TextScale.X = mCamera->TextScale.Y = 1.0f;
	//GEngine->AddOnScreenDebugMessage(4, 10, FColor::White, "Current Ability                   : " + FString::FromInt(currentAbility)); 	FScreenMessageString* mCurrentAbility = GEngine->ScreenMessages.Find(4); 	mCurrentAbility->TextScale.X = mCurrentAbility->TextScale.Y = 1.0f;
	//GEngine->AddOnScreenDebugMessage(5, 10, FColor::White, "Can Choose Ability                : " + FString::Printf(TEXT("%s"), canChooseAbility ? TEXT("yes") : TEXT("no"))); 	FScreenMessageString* mcanChooseAbility = GEngine->ScreenMessages.Find(5); 	mcanChooseAbility->TextScale.X = mcanChooseAbility->TextScale.Y = 1.0f;
	//GEngine->AddOnScreenDebugMessage(6, 10, FColor::White, "Can Use Ability                   : " + FString::Printf(TEXT("%s"), canUseAbility? TEXT("yes") : TEXT("no"))); 	FScreenMessageString* mCanUseAbility = GEngine->ScreenMessages.Find(6); 	mCanUseAbility->TextScale.X = mCanUseAbility->TextScale.Y = 1.0f;
	//GEngine->AddOnScreenDebugMessage(7, 10, FColor::White, "Can Execute Ability               : " + FString::Printf(TEXT("%s"), canExecuteAbility ? TEXT("yes") : TEXT("no"))); 	FScreenMessageString* mCanExecuteAbility = GEngine->ScreenMessages.Find(7); 	mCanExecuteAbility->TextScale.X = mCanExecuteAbility->TextScale.Y = 1.0f;
	//GEngine->AddOnScreenDebugMessage(8, 10, FColor::White, "Is Using Ability                  : " + FString::Printf(TEXT("%s"), isUsingAbility ? TEXT("yes") : TEXT("no")));     FScreenMessageString* mIsUsingAbility = GEngine->ScreenMessages.Find(8); 	mIsUsingAbility->TextScale.X = mIsUsingAbility->TextScale.Y = 1.0f;
	//GEngine->AddOnScreenDebugMessage(9, 10, FColor::White, "Is Executing Ability              : " + FString::Printf(TEXT("%s"), isExecutingAbility ? TEXT("yes") : TEXT("no"))); 	FScreenMessageString* mIsExecutingAbility = GEngine->ScreenMessages.Find(9); 	mIsExecutingAbility->TextScale.X = mIsExecutingAbility->TextScale.Y = 1.0f;
	//GEngine->AddOnScreenDebugMessage(10, 10, FColor::White, "Is Selecting Warp Location        : " + FString::Printf(TEXT("%s"), isChoosingWarpDestination ? TEXT("yes") : TEXT("no"))); FScreenMessageString* mIsSelectingWarpLocation = GEngine->ScreenMessages.Find(10); mIsSelectingWarpLocation->TextScale.X = mIsSelectingWarpLocation->TextScale.Y = 1.0f;
	//GEngine->AddOnScreenDebugMessage(11, 10, FColor::White, "Is Selecting Slow Time              : " + FString::Printf(TEXT("%s"), isChoosingSlowTime ? TEXT("yes") : TEXT("no"))); FScreenMessageString* mIsSelectingSlowTime = GEngine->ScreenMessages.Find(11); mIsSelectingSlowTime->TextScale.X = mIsSelectingSlowTime->TextScale.Y = 1.0f;
	//GEngine->AddOnScreenDebugMessage(12, 10, FColor::White, "Is Selecting Wall Target          : " + FString::Printf(TEXT("%s"), isChoosingWallrideObject ? TEXT("yes") : TEXT("no"))); FScreenMessageString* mIsSelectingWallTarget = GEngine->ScreenMessages.Find(12); mIsSelectingWallTarget->TextScale.X = mIsSelectingWallTarget->TextScale.Y = 1.0f;
	//GEngine->AddOnScreenDebugMessage(14, 10, FColor::White, "GLobal Ability Cooldown Active    : " + FString::Printf(TEXT("%s"), isGlobalAbilityCoolingDown ? TEXT("yes") : TEXT("no"))); FScreenMessageString* mGlobalCooldownActive = GEngine->ScreenMessages.Find(14); mGlobalCooldownActive->TextScale.X = mGlobalCooldownActive->TextScale.Y = 1.0f;
	//GEngine->AddOnScreenDebugMessage(15, 10, FColor::White, "Global Cooldown Remaining         : " + FString::SanitizeFloat(globalCooldownTimeRemaining)); FScreenMessageString* mGlobalCooldownRemaining = GEngine->ScreenMessages.Find(15); mGlobalCooldownRemaining->TextScale.X = mGlobalCooldownRemaining->TextScale.Y = 1.0f;
    //GEngine->AddOnScreenDebugMessage(16, 10, FColor::White, "Warp Cooldown Active              : " + FString::Printf(TEXT("%s"), isWarpCoolingDown ? TEXT("yes") : TEXT("no"))); FScreenMessageString* mWarpCooldownActive = GEngine->ScreenMessages.Find(16); mWarpCooldownActive->TextScale.X = mWarpCooldownActive->TextScale.Y = 1.0f;
	//GEngine->AddOnScreenDebugMessage(17, 10, FColor::White, "Warp Cooldown Remaining           : " + FString::SanitizeFloat(warpCooldownTimeRemaining)); FScreenMessageString* mWarpCooldownRemaining = GEngine->ScreenMessages.Find(17); mWarpCooldownRemaining->TextScale.X = mWarpCooldownRemaining->TextScale.Y = 1.0f;
	//GEngine->AddOnScreenDebugMessage(18, 10, FColor::White, "Slow Cooldown Active              : " + FString::Printf(TEXT("%s"), isSlowCoolingDown ? TEXT("yes") : TEXT("no"))); FScreenMessageString* mSlowCooldownActive = GEngine->ScreenMessages.Find(18); mSlowCooldownActive->TextScale.X = mSlowCooldownActive->TextScale.Y = 1.0f;
	//GEngine->AddOnScreenDebugMessage(19, 10, FColor::White, "Slow Cooldown Remaining           : " + FString::SanitizeFloat(slowCooldownTimeRemaining)); FScreenMessageString* mSlowCooldownRemaining = GEngine->ScreenMessages.Find(19); mSlowCooldownRemaining->TextScale.X = mSlowCooldownRemaining->TextScale.Y = 1.0f;
	//GEngine->AddOnScreenDebugMessage(20, 10, FColor::White, "Wall Cooldown Active              : " + FString::Printf(TEXT("%s"), isWallrideCoolingDown ? TEXT("yes") : TEXT("no"))); FScreenMessageString* mWallCooldownActive = GEngine->ScreenMessages.Find(20); mWallCooldownActive->TextScale.X = mWallCooldownActive->TextScale.Y = 1.0f;
	//GEngine->AddOnScreenDebugMessage(21, 10, FColor::White, "Wall Cooldown Remaining           : " + FString::SanitizeFloat(wallRideCooldownTimeRemaining)); FScreenMessageString* mWallCooldownRemaining = GEngine->ScreenMessages.Find(21); mWallCooldownRemaining->TextScale.X = mWallCooldownRemaining->TextScale.Y = 1.0f;
	//GEngine->AddOnScreenDebugMessage(24, 10, FColor::White, "Current Gear                      : " + FString::FromInt(VehicleMovement->GetCurrentGear())); FScreenMessageString* mCurrentGear = GEngine->ScreenMessages.Find(24); mCurrentGear->TextScale.X = mCurrentGear->TextScale.Y = 1.0f;
	//GEngine->AddOnScreenDebugMessage(25, 10, FColor::White, "Speed                               : " + FString::SanitizeFloat(VehicleMovement->GetForwardSpeed() * .036)); FScreenMessageString* mRPM = GEngine->ScreenMessages.Find(25); mRPM->TextScale.X = mRPM->TextScale.Y = 1.0f;
	//GEngine->AddOnScreenDebugMessage(26, 10, FColor::White, "Is Wallriding                       : " + FString::Printf(TEXT("%s"), isWallriding ? TEXT("yes") : TEXT("no"))); FScreenMessageString* mIsWallriding = GEngine->ScreenMessages.Find(26); mIsWallriding->TextScale.X = mIsWallriding->TextScale.Y = 1.0f;
	//GEngine->AddOnScreenDebugMessage(27, 10, FColor::White, "Is Slow Active                      : " + FString::Printf(TEXT("%s"), isSlowActive ? TEXT("yes") : TEXT("no"))); FScreenMessageString* mIsSlowActive = GEngine->ScreenMessages.Find(27); mIsSlowActive->TextScale.X = mIsSlowActive->TextScale.Y = 1.0f;
	//GEngine->AddOnScreenDebugMessage(28, 10, FColor::White, "Slow Active Time                      : " + FString::SanitizeFloat(slowActiveTime) + "/" + FString::SanitizeFloat(slowActiveMaxTime)); FScreenMessageString* mSlowActiveTime = GEngine->ScreenMessages.Find(28); mSlowActiveTime->TextScale.X = mSlowActiveTime->TextScale.Y = 1.0f;
}

#pragma endregion Tick

#pragma region Car operation

void APlayerCarPawn::MoveForward(float f)
{
	if (VehicleMovement && !isFindingUpright && hasLevelStarted && !isPaused)
	{
		if (isPhysicsNormal)
		{
			VehicleMovement->SetThrottleInput(f);
		}
	}
}

void APlayerCarPawn::MoveRight(float f)
{
	if (VehicleMovement  && !isFindingUpright && hasLevelStarted && !isPaused)
	{
		if (isPhysicsNormal)
		{
			VehicleMovement->SetSteeringInput(f);
		}
	}
}

void APlayerCarPawn::Turn(float f)
{

}

void APlayerCarPawn::HandBrakePressed()
{
	if (VehicleMovement && hasLevelStarted)
	{
		VehicleMovement->SetHandbrakeInput(true);
	}
}

void APlayerCarPawn::HandBrakeReleased()
{
	if (VehicleMovement && !isEnteringLevel)
	{
		VehicleMovement->SetHandbrakeInput(false);
	}
}

void APlayerCarPawn::GearUp()
{
	if (VehicleMovement->GetCurrentGear() < 5 && hasLevelStarted)
	{
		//VehicleMovement->SetGearUp(true);
		VehicleMovement->SetTargetGear(VehicleMovement->GetCurrentGear() + 1, true);
		UpdateSelectedGearUI();
	}
}

void APlayerCarPawn::GearDown()
{
	if (VehicleMovement->GetCurrentGear() > 1 && hasLevelStarted)
	{
		//VehicleMovement->SetGearDown(true);
		VehicleMovement->SetTargetGear(VehicleMovement->GetCurrentGear() - 1, true);
		PlayShiftDownSoundCue();
		UpdateSelectedGearUI();
	}
}

void APlayerCarPawn::UpdateSelectedGearUI()
{
	switch (VehicleMovement->GetCurrentGear())
	{
	case -1:
		UIWidget->UpdateGearText(FString("R"));
		break;
	case 0:
		UIWidget->UpdateGearText(FString("N"));
		break;
	case 1:
		UIWidget->UpdateGearText(FString("1"));
		break;
	case 2:
		UIWidget->UpdateGearText(FString("2"));
		break;
	case 3:
		UIWidget->UpdateGearText(FString("3"));
		break;
	case 4:
		UIWidget->UpdateGearText(FString("4"));
		break;
	case 5:
		UIWidget->UpdateGearText(FString("5"));
		break;
	default:
		break;
	}
}

void APlayerCarPawn::StartToFindUpright()
{
	float carWorldRotationRoll = CarMesh->GetComponentRotation().Roll;
	float carWorldRotationPitch = CarMesh->GetComponentRotation().Pitch;

	if (carWorldRotationRoll > 43.f && CarMesh->GetComponentVelocity().Size() < 2.f)
	{
		isFindingUpright = true;
	}
	else if (carWorldRotationRoll < -43.f && CarMesh->GetComponentVelocity().Size() < 2.f)
	{
		isFindingUpright = true;
	}
	else if (carWorldRotationPitch > 45.f && CarMesh->GetComponentVelocity().Size() < 2.f)
	{
		isFindingUpright = true;
	}
	else if (carWorldRotationPitch < -45.f && CarMesh->GetComponentVelocity().Size() < 2.f)
	{
		isFindingUpright = true;
	}
}

void APlayerCarPawn::AddHealth(float f)
{
	if (currentHealth < maxHealth)
	{
		currentHealth += f;
		currentHealth > maxHealth ? currentHealth = maxHealth : currentHealth = currentHealth;
	}
}

void APlayerCarPawn::AutoHealHealth()
{
	if (currentHealth < maxHealth)
	{
		currentHealth += autoHealRate;
		currentHealth > maxHealth ? currentHealth = maxHealth : currentHealth = currentHealth;
	}
}

#pragma endregion Car operation

#pragma region Camera Operation

void APlayerCarPawn::TurnCamera(float f)
{
	if (!isUsingAbility && !isWallriding && !isResettingRoll && hasLevelStarted && !isPaused)
	{
		if (isThidPersonCameraActive)
		{
			thirdPersonSpringArm->SetRelativeRotation(FRotator(thirdPersonSpringArm->GetRelativeTransform().Rotator().Pitch, thirdPersonSpringArm->GetRelativeTransform().Rotator().Yaw + f, thirdPersonSpringArm->GetRelativeTransform().Rotator().Roll));
		}
		else if (isFirstPersonCameraActive)
		{
			firstPersonCamera->AddLocalRotation(FRotator(0.f, f, 0.f));
		}
	}
	else if (isUsingAbility && currentAbility == 2)
	{
		if (isThidPersonCameraActive)
		{
			thirdPersonSpringArm->SetRelativeRotation(FRotator(thirdPersonSpringArm->GetRelativeTransform().Rotator().Pitch, thirdPersonSpringArm->GetRelativeTransform().Rotator().Yaw + f, thirdPersonSpringArm->GetRelativeTransform().Rotator().Roll));
		}
		else if (isFirstPersonCameraActive)
		{
			firstPersonCamera->AddLocalRotation(FRotator(0.f, f, 0.f));
		}
	}
}

void APlayerCarPawn::SwitchCamera()
{
	if (canSwitchCamera && hasLevelStarted && !isPaused)
	{
		ResetCamera();
		if (isThidPersonCameraActive)
		{
			thirdPersonCamera->SetActive(false);
			isThidPersonCameraActive = false;
			firstPersonCamera->SetActive(true);
			isFirstPersonCameraActive = true;
		}
		else if (isFirstPersonCameraActive)
		{
			firstPersonCamera->SetActive(false);
			isFirstPersonCameraActive = false;
			thirdPersonCamera->SetActive(true);
			isThidPersonCameraActive = true;
		}
	}
}

void APlayerCarPawn::ResetCamera()
{
	if (hasLevelStarted)
	{
		thirdPersonSpringArm->SetRelativeRotation(FRotator(-10.f, 0.f, 0.f));
		firstPersonCamera->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	}
}

#pragma endregion Camera Operation

#pragma region Sound

UAudioComponent* APlayerCarPawn::playSoundCue(USoundCue* sound)
{
	return CreateDefaultSubobject<UAudioComponent>("Audio Component");
}

void APlayerCarPawn::ChangeAndPlayAbilitySoundCue(USoundCue* cue)
{
	abilityAudioComponent->SetSound(cue);
	abilityAudioComponent->Play();
}

void APlayerCarPawn::StopAbilitySoundCue()
{
	if (abilityAudioComponent->IsPlaying())
	{
		abilityAudioComponent->Stop();
	}
}

void APlayerCarPawn::PlayLevelSound(USoundCue* cue)
{
	levelInteractionAudioComponent->SetSound(cue);
	levelInteractionAudioComponent->Play();
}

void APlayerCarPawn::PlayImpactSoundCue()
{
	impactAudioComponent->SetSound(impactAudio);
	impactAudioComponent->Play();
}

void APlayerCarPawn::PlayShiftDownSoundCue()
{
	engineMiscAudioComponent->SetSound(shiftDownAudio);
	engineMiscAudioComponent->Play();
}

void APlayerCarPawn::PlayOffThrottleSoundCue()
{
	if (engineMiscAudioComponent->IsPlaying())
	{
		engineMiscAudioComponent->Stop();
	}
	engineMiscAudioComponent->SetSound(offThrottleAudio);
	engineMiscAudioComponent->Play();
}

#pragma endregion Sound

#pragma region Ability Operation

void APlayerCarPawn::AbilityNext()
{
	if (canChooseAbility && hasLevelStarted && !isPaused)
	{
		if (currentAbility < 3)
		{
			currentAbility += 1;
			UpdateSelectedAbilityUI();
		}
	}
}

void APlayerCarPawn::AbilityPrevious()
{
	if (canChooseAbility && hasLevelStarted && !isPaused)
	{
		if (currentAbility > 1)
		{
			currentAbility -= 1;
			UpdateSelectedAbilityUI();
		}
	}
}

void APlayerCarPawn::Ability(int choice)
{
	if (canChooseAbility && hasLevelStarted && !isPaused)
	{
		switch (choice)
		{
		case 1:
			currentAbility = 1;
			UpdateSelectedAbilityUI();
			break;
		case 2:
			currentAbility = 2;
			UpdateSelectedAbilityUI();
			break;
		case 3:
			currentAbility = 3;
			UpdateSelectedAbilityUI();
			break;
		default:
			break;
		}
	}
}

void APlayerCarPawn::UpdateSelectedAbilityUI()
{
	switch (currentAbility)
	{
	case 1:
		UIWidget->UpdateAbilityText(FString("WARP"));
		break;
	case 2:
		UIWidget->UpdateAbilityText(FString("SLOWTIME"));
		break;
	case 3:
		UIWidget->UpdateAbilityText(FString("WALLRIDE"));
		break;
	default:
		UIWidget->UpdateAbilityText(FString(""));
		break;
	}
}

void APlayerCarPawn::AbilityActivate()
{
	if (canUseAbility && !isGlobalAbilityCoolingDown && hasLevelStarted && !isPaused)
	{
		switch (currentAbility)
		{
		case 1: // Warp
			if (!isWarpCoolingDown && !isCastPointOverlapped && currentEnergy >= energyRequiredWarp)
			{
				ResetCamera();
				canChooseAbility = false;
				isUsingAbility = true;
				warpTargetBox->bGenerateOverlapEvents = true;
				WarpSelectModeActive();
				canExecuteAbility = true;
			}
			break;
		case 2: // Slow
			if (!isSlowCoolingDown && currentEnergy >= energyRequiredSlow)
			{
				canChooseAbility = false;
				isUsingAbility = true;
				SlowSelectModeActive();
				canExecuteAbility = true;
			}
			break;
		case 3: // Wallride
			if (!isWallrideCoolingDown && !isCastPointOverlapped && currentEnergy >= energyRequiredWallRide)
			{
				ResetCamera();
				canChooseAbility = false;
				isUsingAbility = true;
				wallrideTargetBox->bGenerateOverlapEvents = true;
				WallrideSelectModeActive();
				canExecuteAbility = false;
			}
			break;
		default:
			break;
		}
	}
	else
	{
		// Do ability failed
	}
}

void APlayerCarPawn::AbilityExecute()
{
	if (canExecuteAbility && hasLevelStarted && !isPaused)
	{
		switch (currentAbility)
		{
		case 1: // Warp
			isExecutingAbility = true;
			DoWarp();
			CancelWarpMode();
			break;
		case 2: // Slow
			isExecutingAbility = true;
			DoSlow();
			break;
		case 3: // Wallride
			isExecutingAbility = true;
			DoWallRide();
			break;
		default:
			break;
		}
	}
	else if (hasLevelStarted && !isPaused)
	{
		switch (currentAbility)
		{
		case 1:
			if (isUsingAbility)
			{
				CancelWarpMode();
				HandleAbility1Animation();
				CancelAnimation();
				MoveLunaInside();
			}
			else
			{
				CancelWarpMode();
			}
			break;
		case 2:
			if (isUsingAbility)
			{
				CancelSlowMode();
				HandleAbility2Animation();
			}
			break;
		case 3:
			if (!isWallriding && isUsingAbility)
			{
				CancelWallrideMode();
				HandleAbility3Animation();
				CancelAnimation();
				MoveLunaInside();
			}
			break;
		default:
			break;
		}
	}
	UIWidget->UpdateEnergyBar(currentEnergy / maxEnergy);
}

void APlayerCarPawn::AbilityCancel()
{
	if (isUsingAbility && !isExecutingAbility && hasLevelStarted && !isPaused)
	{
		switch (currentAbility)
		{
		case 1: // Warp
			CancelWarpMode();
			CancelAnimation();
			MoveLunaInside();
			warpTargetBox->bGenerateOverlapEvents = false;
			break;
		case 2: // Slow
		{
			CancelSlowMode();
			CancelAnimation();
			break;
		}
		case 3: // Wallride
			if (!isWallriding)
			{
				CancelWallrideMode();
				CancelAnimation();
				MoveLunaInside();
				wallrideTargetBox->bGenerateOverlapEvents = false;
			}
			break;
		default:
			break;
		}
	}
	// Special case, stop wallriding after starting
	else if (isWallriding)
	{
		AbortWallride();
		wallrideTargetBox->bGenerateOverlapEvents = false;
	}
	// Special case, stop slow after starting
	else if (isSlowActive)
	{
		CancelSlowMode();
	}
}

void APlayerCarPawn::RescueAbility()
{
	if (!isUsingAbility && !isExecutingAbility && !isGlobalAbilityCoolingDown && !isWallriding && !isFindingUpright)
	{
		StartToFindUpright();
	}
}

void APlayerCarPawn::AddEnergy(float f)
{
	if (currentEnergy < maxEnergy)
	{
		currentEnergy += f;
		currentEnergy > maxEnergy ? currentEnergy = maxEnergy : currentEnergy = currentEnergy;
	}
}

void APlayerCarPawn::AutoRechargeEnergy()
{
	if (currentEnergy < maxEnergy)
	{
		currentEnergy += autoRechargeRate;
		currentEnergy > maxEnergy ? currentEnergy = maxEnergy : currentEnergy;
		UIWidget->UpdateEnergyBar(currentEnergy / maxEnergy);
	}
}

#pragma endregion Ability Operation

#pragma region Ability 1

// Player is choosing the desired location to warp to
void APlayerCarPawn::WarpSelectModeActive()
{
	ChangeAndPlayAbilitySoundCue(ability1LoopAudio);
	PlayWarpParticles();

	// This flag allows for the warpTargetBox to have its position calculated
	isChoosingWarpDestination = true;
	// Force third person perspective, prevent player from switching back to first person
	canSwitchCamera = false;

	// If the player is using first person camera, shift to third person, do not set the flag
	// we use this flag to know we need to shift back to the desired camera after the warp is over
	if (isFirstPersonCameraActive)
	{
		thirdPersonCamera->SetActive(true);
		firstPersonCamera->SetActive(false);
	}

	// Rotate the camera spring arm to a location that makes it easier to see
	thirdPersonSpringArm->SetRelativeRotation(FRotator(-34.f, 0.f, 0.f));

	// Have luna start to cast and move her outside
	MoveLunaToCastPoint();
	HandleAbility1Animation();
}

// Execute the warp action
void APlayerCarPawn::DoWarp()
{
	currentEnergy -= energyRequiredWarp;

	PlayWarpExecuteParticles();

	// We are no longer selecting the location
	isUsingAbility = false;
	isChoosingWarpDestination = false;

	// Have luna execute the cast, but cannot move her back inside yet
	// Must wait until cast animation finishes before placing her inside
	HandleAbility1Animation();
	isWaitingForAnimationToEnd = true;

	// Set the location of the car to the warpTargetBox
	// The warp target box has been moved above the ground to check for collision so move the teleport target back to the ground
	CarMesh->SetWorldLocationAndRotation(warpTargetBox->GetComponentLocation() - warpTargetBox->GetUpVector() * 75, warpTargetBox->GetComponentRotation(), false, (FHitResult *)nullptr, ETeleportType::TeleportPhysics);

	// Set cooldown active for both global abilities and warp
	isWarpCoolingDown = true;
	isGlobalAbilityCoolingDown = true;

	warpCooldownTimeRemaining = warpCooldownMax;
	globalCooldownTimeRemaining = globalCooldownMax;
}

// Cancel the warp by player choice, or when the warp was unable to execute
// Cooldown only applies when warp succeeds, no penalty for canceling ability
void APlayerCarPawn::CancelWarpMode()
{
	StopAbilitySoundCue();
	DestroyWarpParticles();

	// We are no longer choosing the location and need to reset ability choosing booleans
	isUsingAbility = false;
	isChoosingWarpDestination = false;
	isExecutingAbility = false;
	canExecuteAbility = false;
	foundGroundAbove = false;
	canChooseAbility = true;

	// Reset the rotation of the warpTargeArm
	warpRotationAngle = 180;

	// If the player was forced into third person camera the flag for
	// isFirstPersonCameraActive will stil be true, so we need to shift
	// the camera back to first person
	if (isFirstPersonCameraActive)
	{
		firstPersonCamera->SetActive(true);
		thirdPersonCamera->SetActive(false);
	}

	// Allow for player driven camera switching again
	canSwitchCamera = true;

	// Rotate the third person camera back to normal perspective
	thirdPersonSpringArm->SetRelativeRotation(FRotator(-10.f, thirdPersonSpringArm->GetRelativeTransform().Rotator().Yaw, thirdPersonSpringArm->GetRelativeTransform().Rotator().Roll));
	// Rotate the warp target arm back to normal rotation
	warpTargetArm->SetRelativeRotation(FRotator(0.f, warpRotationAngle, 0.f));
}

// Move the warpTargetArm up to a maximum of 45 degrees more or less than 180 degrees
// Only valid if isChoosingWarpDestination is true
// if f is positive increase rotation, if f is negative decrease rotation
void APlayerCarPawn::WarpDestinationMoveRight(float f)
{
	if (isChoosingWarpDestination)
	{
		if (f > 0 && warpRotationAngle < 225.f)
		{
			warpRotationAngle += warpRotateAtRate * f;
			warpTargetArm->SetRelativeRotation(FRotator(0.f, warpRotationAngle, 0.f));
		}
		else if (f < 0 && warpRotationAngle > 135.f)
		{
			warpRotationAngle += warpRotateAtRate * f;
			warpTargetArm->SetRelativeRotation(FRotator(0.f, warpRotationAngle, 0.f));
		}
	}
}

// Search for ground above or below warp arm, precedence given to ground above
void APlayerCarPawn::PrepareGroundProbe(bool up)
{
	// Set the direction of the GroundProbe
	float direction;
	up ? direction = 1 : direction = -1;

	// Getr start and end locations for the GroundProbe
	FVector groundProbeStartLocation = warpTargetArm->GetSocketLocation(USpringArmComponent::SocketName);
	FVector groundProbeDirection = direction * warpTargetArm->GetUpVector();
	FVector groundProbeEndLocation = groundProbeStartLocation + groundProbeDirection * 500.f;

	// Execute the GroundProbe
	GroundProbe(groundProbeStartLocation, groundProbeEndLocation, up);
}

// Execute GroundProbe from start location to end location
// Up indicates search direction, if up Start and End points are switched
// to start above the End location and search down towards the Start
FHitResult APlayerCarPawn::GroundProbe(const FVector &Start, const FVector &End, bool up)
{
	FHitResult HitOut;

	if (GetWorld())
	{
		bool hit;
		FHitResult HitOut(ForceInit);

		FCollisionQueryParams TraceParams("Ability1Trace", true, this);
		// IF up switch Start and End points, use camera collision channel to hit all objects
		if (up)
		{
			hit = GetWorld()->LineTraceSingleByChannel(HitOut, End, Start, ECollisionChannel::ECC_GameTraceChannel1, TraceParams);
		}
		else
		{
			hit = GetWorld()->LineTraceSingleByChannel(HitOut, Start, End, ECollisionChannel::ECC_GameTraceChannel1, TraceParams);
		}

		// If blocking hit is found
		if (hit)
		{

			// roll.pitch gives the angle between the ImpactNormal and the RightVector of the CarMesh
			FRotator roll = UKismetMathLibrary::MakeRotFromXZ(HitOut.ImpactNormal, CarMesh->GetRightVector() + (CarMesh->GetRightVector() * 90));

			if (roll.Pitch > 45.f)
			{
				// If we are searching up, we are not going to also search down this frame
				if (up)
				{
					foundGroundAbove = true;
				}
				// Move the warpTargetBox to the location of the ImpactPoint and orient it with relation to the ImpactNormal
				warpTargetBox->SetWorldLocationAndRotation(HitOut.ImpactPoint + HitOut.ImpactNormal * 75, UKismetMathLibrary::MakeRotFromZX(HitOut.ImpactNormal, CarMesh->GetForwardVector() + (CarMesh->GetForwardVector() * 90)));
				// This indicates that we have found a valid destination in terms of location, we still need to check for overlaps
				canExecuteAbility = true;
			}
			else
			{
				// If we started by searching up, we need to indicate that we also need to search down this frame
				if (up)
				{
					foundGroundAbove = false;
				}
				// This indicates that we have not found a valid destination in terms of location
				canExecuteAbility = false;
			}
		}
		else // No blocking hit was found
		{
			// If we started by searching up, we need to indicate that we also need to search down this frame
			if (up)
			{
				foundGroundAbove = false;
			}
			// This indicates that we have not found a valid destination in terms of location
			canExecuteAbility = false;
		}
	}
	return HitOut;  // Unused
}

// Check for overlaps of the warpTargetBox, any overlaps indicate that the current destination is invalid
void APlayerCarPawn::WarpDestinationCheckOverlap()
{
	// Get all overlappingActors
	TArray<AActor*> overlappingActors;
	overlappingActors.Reset();
	warpTargetBox->GetOverlappingActors(overlappingActors);

	// If there are any overlappingActors, this is an invalid destination
	if (overlappingActors.Num() != 0)
	{
		canExecuteAbility = false;
		if (isPlayingValidLocationParticleSystem)
		{
			AdjustWarpParticles(false);
		}
	}
	else
	{
		canExecuteAbility = true;
		if (!isPlayingValidLocationParticleSystem)
		{
			AdjustWarpParticles(true);
		}
	}

}

#pragma endregion Warp Ability Operation

#pragma region Ability 2

void APlayerCarPawn::SlowSelectModeActive()
{
	ChangeAndPlayAbilitySoundCue(ability2LoopAudio);

	isChoosingSlowTime = true;

	HandleAbility2Animation();
}

void APlayerCarPawn::CancelSlowMode()
{
	StopAbilitySoundCue();

	isUsingAbility = false;
	isChoosingSlowTime = false;
	isExecutingAbility = false;
	canExecuteAbility = false;
	canChooseAbility = true;
	canUseAbility = true;

	if (isSlowActive)
	{
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
		this->CustomTimeDilation = 1.f;

		SetNormalMovement();

		if (CarMesh->GetPhysicsLinearVelocity().Size() > 2500.f)
		{
			CarMesh->SetPhysicsLinearVelocity(2500.f * CarMesh->GetForwardVector());
		}
		else
		{
			CarMesh->SetPhysicsLinearVelocity(CarMesh->GetComponentVelocity().Size() * CarMesh->GetForwardVector());
		}
		CarMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

		ChangeAndPlayAbilitySoundCue(ability2ExitAudio);
		PlaySlowExecuteParticles();

		isSlowCoolingDown = true;
		isGlobalAbilityCoolingDown = true;

		slowCooldownTimeRemaining = slowCooldownMax;
		globalCooldownTimeRemaining = globalCooldownMax;
	}

	isSlowActive = false;
	isPhysicsNormal = true;

	slowActiveTime = 0.f;

	thirdPersonCamera->SetFieldOfView(100.f);
	firstPersonCamera->SetFieldOfView(100.f);
}

void APlayerCarPawn::DoSlow()
{
	currentEnergy -= energyRequiredSlow;

	isUsingAbility = false;
	isChoosingSlowTime = false;
	canChooseAbility = false;
	canUseAbility = false;
	canExecuteAbility = false;

	HandleAbility2Animation();
	isWaitingForAnimationToEnd = true;

	isPhysicsNormal = false;
	isSlowActive = true;

	thirdPersonCamera->SetFieldOfView(120.f);
	firstPersonCamera->SetFieldOfView(120.f);

	SetSlowMovement();
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), .1);
	this->CustomTimeDilation =  10.f;

	isExecutingAbility = false;
}

void APlayerCarPawn::SetSlowMovement()
{
	VehicleMovement->Deactivate();
}

void APlayerCarPawn::SetNormalMovement()
{
	VehicleMovement->Activate();
}

void APlayerCarPawn::SlowMovement(float dTime)
{
	float forwardValue = GetInputAxisValue("MoveForward");
	float rightValue = GetInputAxisValue("TurnRight");

	FVector rawInputMove = FVector(forwardValue, rightValue, 0.f);
	rawInputMove = UKismetMathLibrary::ClampVectorSize(rawInputMove, 0.f, 1.f);
	FVector speedAdjustedMove = rawInputMove * (FVector(300, 300, 0) * dTime);

	AddActorLocalOffset(speedAdjustedMove, false, (FHitResult *)nullptr, ETeleportType::TeleportPhysics);
	AddActorLocalRotation(FRotator(0.f, rightValue, 0.f), false, (FHitResult *)nullptr, ETeleportType::TeleportPhysics);
}

#pragma endregion Slow Ability Operation

#pragma region Ability 3

void APlayerCarPawn::WallrideSelectModeActive()
{
	ChangeAndPlayAbilitySoundCue(ability3LoopAudio);
	PlayWallrideParticles();

	isChoosingWallrideObject = true;

	canSwitchCamera = false;

	if (isFirstPersonCameraActive)
	{
		thirdPersonCamera->SetActive(true);
		firstPersonCamera->SetActive(false);
	}

	// Have luna start to cast and move her outside
	MoveLunaToCastPoint();
	HandleAbility3Animation();
}

void APlayerCarPawn::CancelWallrideMode()
{
	StopAbilitySoundCue();
	DestroyWallrideParticles();

	isUsingAbility = false;
	isChoosingWallrideObject = false;
	isExecutingAbility = false;
	canExecuteAbility = false;;
	canChooseAbility = true;

	if (ability3TargetActor)
	{
		ability3TargetActor->DoMaterialReset();
	}

	if (isFirstPersonCameraActive)
	{
		firstPersonCamera->SetActive(true);
		thirdPersonCamera->SetActive(false);
	}

	canSwitchCamera = true;

	wallrideRotationAngle = 0.f;
	wallrideTargetArm->SetRelativeRotation(FRotator(0.f, 145.f, 0.f));
}

void APlayerCarPawn::DoWallRide()
{
	currentEnergy -= energyRequiredWallRide;

	ChangeAndPlayAbilitySoundCue(ability3WallrideAudio);
	// We are no longer selecting the location
	isUsingAbility = false;
	isChoosingWallrideObject = false;
	canChooseAbility = false;
	canUseAbility = false;
	canExecuteAbility = false;

	HandleAbility3Animation();
	isWaitingForAnimationToEnd = true;

	DestroyWallrideParticles();
	PlayWallrideExecuteParticles();

	// Set the location of the car to the warpTargetBox
	// The warp target box has been moved above the ground to check for collision so move the teleport target back to the ground
	CarMesh->SetWorldLocationAndRotation(wallrideTargetBox->GetComponentLocation() - wallrideTargetBox->GetUpVector() * 75, wallrideTargetBox->GetComponentRotation(), false, (FHitResult *)nullptr, ETeleportType::TeleportPhysics);
	isExecutingAbility = false;

	wallrideTargetBox->bGenerateOverlapEvents = false;

	SetWallrideGravity();

	isWallriding = true;
	isPhysicsNormal = false;
}

void APlayerCarPawn::WallrideDestinationMoveRight(float f)
{
	if (isChoosingWallrideObject)
	{
		wallrideRotationAngle = wallrideRotateAtRate * f;
		wallrideTargetArm->AddWorldRotation(FQuat(CarMesh->GetForwardVector(), UKismetMathLibrary::DegreesToRadians(wallrideRotationAngle)));
	}
}

void APlayerCarPawn::FindWallToRide()
{
	// Get all overlappingActors

	TArray<AActor*> ability3ObjectArray;
	ability3ObjectArray.Reset();
	wallrideArmSphere->GetOverlappingActors(ability3ObjectArray);

	if (ability3ObjectArray.Num() > 0)
	{
		ability3TargetActor = CastChecked<AAbility3Actor>(ability3ObjectArray[0]);
		if (ability3TargetActor->isAlreadyOverlapped)
		{
			ability3TargetActor->DoMaterialChange();
		}
		// Viable wall found, prepare the wall probe
		PrepareWallProbe();
	}
	else
	{
		canExecuteAbility = false;
	}
}

void APlayerCarPawn::PrepareWallProbe()
{

	// Getr start and end locations for the WallProbe
	FVector wallProbeStartLocation = wallrideTargetArm->GetSocketLocation(USpringArmComponent::SocketName);
	FVector wallProbeDirection = wallrideTargetArm->GetRightVector();
	FVector wallProbeEndLocation = wallProbeStartLocation + wallProbeDirection* 300.f;

	// Execute the GroundProbe
	WallProbe(wallProbeStartLocation, wallProbeEndLocation);
}

FHitResult APlayerCarPawn::WallProbe(const FVector &Start, const FVector &End)
{
	FHitResult HitOut;

	if (GetWorld())
	{
		bool hit;
		FHitResult HitOut(ForceInit);

		hit = GetWorld()->LineTraceSingleByChannel(HitOut, Start, End, ECollisionChannel::ECC_GameTraceChannel3);

		// If blocking hit is found
		if (hit)
		{
			wallrideTargetBox->SetWorldLocationAndRotation(HitOut.ImpactPoint + HitOut.ImpactNormal * 75, UKismetMathLibrary::MakeRotFromZX(HitOut.ImpactNormal, CarMesh->GetForwardVector() + (CarMesh->GetForwardVector() * 90)));
			canExecuteAbility = true;
		}
		else // No blocking hit was found
		{
			canExecuteAbility = false;
		}
	}
	return HitOut;  // Unused
}

void APlayerCarPawn::WallrideDestinationCheckOverlap()
{
	// Get all overlappingActors
	TArray<AActor*> overlappingActors;
	overlappingActors.Reset();
	wallrideTargetBox->GetOverlappingActors(overlappingActors);

	// If there are any overlappingActors, this is an invalid destination
	if (overlappingActors.Num() > 1)
	{
		canExecuteAbility = false;
		if (isPlayingValidWallParticleSystem)
		{
			AdjustWallrideParticles(false);
		}
	}
	else
	{
		canExecuteAbility = true;
		if (!isPlayingValidWallParticleSystem)
		{
			AdjustWallrideParticles(true);
		}
	}
}

void APlayerCarPawn::SetWallrideGravity()
{
	CarMesh->SetSimulatePhysics(false);
}

void APlayerCarPawn::SetNormalGravity()
{
	CarMesh->SetSimulatePhysics(true);
}

void APlayerCarPawn::WallrideMovement(float dTime)
{
	float forwardValue = GetInputAxisValue("MoveForward");
	float rightValue = GetInputAxisValue("TurnRight");

	FVector rawInputMove = FVector(forwardValue, rightValue, 0.f);
	rawInputMove = UKismetMathLibrary::ClampVectorSize(rawInputMove, 0.f, 1.f);
	FVector speedAdjustedMove = rawInputMove * (FVector(2750, 2750, 0) * dTime);
	FRotator rotation = speedAdjustedMove.Rotation();

	if (speedAdjustedMove.SizeSquared() > 0.f && forwardValue > 0.f)
	{
		FVector traceStart = CarMesh->GetComponentLocation() + CarMesh->GetUpVector() * 30;
		FVector traceEnd = traceStart + (-CarMesh->GetUpVector() * 100);
		
		FHitResult HitOut;
		bool hit = GetWorld()->LineTraceSingleByChannel(HitOut, traceStart, traceEnd, ECollisionChannel::ECC_GameTraceChannel3);

		if (hit)
		{
			FRotator computed = UKismetMathLibrary::MakeRotFromZX(HitOut.ImpactNormal, CarMesh->GetForwardVector());
			SetActorLocationAndRotation(HitOut.ImpactPoint, computed, false, (FHitResult *)nullptr, ETeleportType::None);
			AddActorLocalOffset(speedAdjustedMove, false, (FHitResult *)nullptr, ETeleportType::None);
		}
		else
		{
			AbortWallride();
		}
	}
	else
	{
		float forwardValue = 0.4f;
		float rightValue = 0.f;

		FVector rawInputMove = FVector(forwardValue, rightValue, 0.f);
		rawInputMove = UKismetMathLibrary::ClampVectorSize(rawInputMove, 0.f, 1.f);
		FVector speedAdjustedMove = rawInputMove * (FVector(2750, 2750, 0) * dTime);
		FRotator rotation = speedAdjustedMove.Rotation();


		FVector traceStart = CarMesh->GetComponentLocation() + CarMesh->GetUpVector() * 30;
		FVector traceEnd = traceStart + (-CarMesh->GetUpVector() * 100);

		FHitResult HitOut;
		bool hit = GetWorld()->LineTraceSingleByChannel(HitOut, traceStart, traceEnd, ECollisionChannel::ECC_GameTraceChannel3);

		if (hit)
		{
			FRotator computed = UKismetMathLibrary::MakeRotFromZX(HitOut.ImpactNormal, CarMesh->GetForwardVector());
			SetActorLocationAndRotation(HitOut.ImpactPoint, computed, false, (FHitResult *)nullptr, ETeleportType::None);
			AddActorLocalOffset(speedAdjustedMove, false, (FHitResult *)nullptr, ETeleportType::None);
		}
		else
		{
			AbortWallride();
		}
	}
}

void APlayerCarPawn::AbortWallride()
{
	ChangeAndPlayAbilitySoundCue(ability3ExitAudio);

	CarMesh->AddLocalOffset(CarMesh->GetUpVector() * 10, false, (FHitResult *)nullptr, ETeleportType::None);

	SetNormalGravity();
	isResettingRoll = true;

	// 2500000
	CarMesh->AddImpulse(CarMesh->GetForwardVector() * 2500000);

	isWallriding = false;
	isPhysicsNormal = true;
	isUsingAbility = false;
	isChoosingWallrideObject = false;
	isExecutingAbility = false;
	canExecuteAbility = false;;
	canUseAbility = true;
	canChooseAbility = true;
	canSwitchCamera = true;

	isWallrideCoolingDown = true;
	wallrideCooldownTimeRemaining = wallrideCooldownMax;

	isGlobalAbilityCoolingDown = true;
	globalCooldownTimeRemaining = globalCooldownMax;

	wallrideRotationAngle = 0.f;
	wallrideTargetArm->SetRelativeRotation(FRotator(0.f, 145.f, 0.f));
}

#pragma endregion Wallride Ability Operation

#pragma region Luna Operation

void APlayerCarPawn::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	isCastPointOverlapped = true;
	if (isChoosingWarpDestination)
	{
		AbilityCancel();
	}
	else if (isChoosingWallrideObject)
	{
		AbilityCancel();
	}
	else if (isWaitingForAnimationToEnd)
	{
		CancelAnimation();
		MoveLunaInside();
	}
}

void APlayerCarPawn::OnOverlapEnd(UPrimitiveComponent* overlappedComp, AActor* otherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	isCastPointOverlapped = false;
}

#pragma endregion Luna Operation

#pragma region Luna Animation

// Operate the animation state machine for ability 1
void APlayerCarPawn::HandleAbility1Animation()
{
	UBoolProperty* isUsingAbilityProperty = FindField<UBoolProperty>(lunaAnimInstance->GetClass(), IS_USING_ABILITY1_PROPERTY);

	if (isUsingAbilityProperty != NULL)
	{
		isUsingAbilityProperty->SetPropertyValue_InContainer(lunaAnimInstance, isUsingAbility);
	}
}

void APlayerCarPawn::HandleAbility2Animation()
{
	UBoolProperty* isUsingAbilityProperty = FindField<UBoolProperty>(lunaAnimInstance->GetClass(), IS_USING_ABILITY2_PROPERTY);

	if (isUsingAbilityProperty != NULL)
	{
		isUsingAbilityProperty->SetPropertyValue_InContainer(lunaAnimInstance, isUsingAbility);
	}
}

void APlayerCarPawn::HandleAbility3Animation()
{
	UBoolProperty* isUsingAbilityProperty = FindField<UBoolProperty>(lunaAnimInstance->GetClass(), IS_USING_ABILITY3_PROPERTY);

	if (isUsingAbilityProperty != NULL)
	{
		isUsingAbilityProperty->SetPropertyValue_InContainer(lunaAnimInstance, isUsingAbility);
	}
}

bool APlayerCarPawn::CheckIfAnimationIsOver()
{
	UBoolProperty* isAnimationOverProperty = FindField<UBoolProperty>(lunaAnimInstance->GetClass(), IS_ANIMATION_OVER_PROPERTY);

	if (isAnimationOverProperty != NULL)
	{
		return isAnimationOverProperty->GetPropertyValue_InContainer(lunaAnimInstance);
	}
	else
	{
		return false;
	}
}

void APlayerCarPawn::CancelAnimation()
{
	UBoolProperty* isCancellingAnimationProperty = FindField<UBoolProperty>(lunaAnimInstance->GetClass(), IS_CANCELLING_ANIMATION_PROPERTY);

	if (isCancellingAnimationProperty != NULL)
	{
		isCancellingAnimationProperty->SetPropertyValue_InContainer(lunaAnimInstance, true);
	}
}

void APlayerCarPawn::MoveLunaToCastPoint()
{
	luna->AttachToComponent(castPointForLuna, FAttachmentTransformRules::SnapToTargetIncludingScale);
}

void APlayerCarPawn::MoveLunaInside()
{
	luna->AttachToComponent(seatForLuna, FAttachmentTransformRules::SnapToTargetIncludingScale);
}

#pragma endregion Luna Animation

#pragma region Particles

void APlayerCarPawn::PlayWarpParticles()
{
	if (isPlayingValidLocationParticleSystem)
	{
		warpArmSphereParticleSystemComponent = UGameplayStatics::SpawnEmitterAttached(warpArmSphereParticleSystemValid, warpArmSphere);
		warpTargetBoxParticleSystemComponent = UGameplayStatics::SpawnEmitterAttached(warpTargetBoxParticleSystemValid, warpTargetBox);
	}
	else
	{
		warpArmSphereParticleSystemComponent = UGameplayStatics::SpawnEmitterAttached(warpArmSphereParticleSystemInvalid, warpArmSphere);
		warpTargetBoxParticleSystemComponent = UGameplayStatics::SpawnEmitterAttached(warpTargetBoxParticleSystemInvalid, warpTargetBox);
	}
}

void APlayerCarPawn::DestroyWarpParticles()
{
	if (warpArmSphereParticleSystemComponent)
	{
		warpArmSphereParticleSystemComponent->DestroyComponent();
	}
	if (warpTargetBoxParticleSystemComponent)
	{
		warpTargetBoxParticleSystemComponent->DestroyComponent();
	}
}

void APlayerCarPawn::AdjustWarpParticles(bool valid)
{
	if (valid)
	{
		isPlayingValidLocationParticleSystem = true;
		DestroyWarpParticles();
		PlayWarpParticles();
	}
	else
	{
		isPlayingValidLocationParticleSystem = false;
		DestroyWarpParticles();
		PlayWarpParticles();
	}
}

void APlayerCarPawn::PlayWarpExecuteParticles()
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), warpExecuteParticleSystem, warpTargetBox->GetComponentTransform(), true);
}

void APlayerCarPawn::PlaySlowExecuteParticles()
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), slowExecuteParticleSystem, GetActorTransform(), true);
}

void APlayerCarPawn::PlayWallrideParticles()
{
	wallrideActiveParticleSystemComponent = UGameplayStatics::SpawnEmitterAttached(wallrideActiveParticleSystem, CarMesh);
	if (isPlayingValidWallParticleSystem)
	{
		wallrideArmSphereParticleSystemComponent = UGameplayStatics::SpawnEmitterAttached(wallrideArmSphereParticleSystemValid, wallrideArmSphere);
		wallrideTargetBoxParticleSystemComponent = UGameplayStatics::SpawnEmitterAttached(wallrideTargetBoxParticleSystemValid, wallrideTargetBox);
	}
	else
	{
		wallrideArmSphereParticleSystemComponent = UGameplayStatics::SpawnEmitterAttached(wallrideArmSphereParticleSystemInvalid, wallrideArmSphere);
		wallrideTargetBoxParticleSystemComponent = UGameplayStatics::SpawnEmitterAttached(wallrideTargetBoxParticleSystemInvalid, wallrideTargetBox);
	}
}

void APlayerCarPawn::DestroyWallrideParticles()
{
	wallrideArmSphereParticleSystemComponent->DestroyComponent();
	wallrideTargetBoxParticleSystemComponent->DestroyComponent();
	wallrideActiveParticleSystemComponent->DestroyComponent();
}

void APlayerCarPawn::AdjustWallrideParticles(bool valid)
{
	if (valid)
	{
		isPlayingValidWallParticleSystem = true;
		DestroyWallrideParticles();
		PlayWallrideParticles();
	}
	else
	{
		isPlayingValidWallParticleSystem = false;
		DestroyWallrideParticles();
		PlayWallrideParticles();
	}
}

void APlayerCarPawn::PlayWallrideExecuteParticles()
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), wallrideExecuteParticleSystem, wallrideTargetBox->GetComponentTransform(), true);
}

#pragma endregion Particles

#pragma region Other Interactions

void APlayerCarPawn::OnHit(UPrimitiveComponent* comp, AActor* otherActor, UPrimitiveComponent* otherComp, FVector hitNormal, const FHitResult &hitResult)
{

}

void APlayerCarPawn::OnPowerSourceOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor != NULL)
	{
		APowerSource* tempPowerSource = CastChecked<APowerSource>(OtherActor);
		if (tempPowerSource->active)
		{
			AddEnergy(tempPowerSource->restoreAmount);
			AddHealth(tempPowerSource->healAmount);
			tempPowerSource->active = false;
		}
	}
}

void APlayerCarPawn::OnPowerSourceOverlapEnd(UPrimitiveComponent* overlappedComp, AActor* otherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

#pragma endregion Other Interactions

#pragma region Level EnterExit

void APlayerCarPawn::StartEnterCountdown()
{
	if (!isEnteringLevel)
	{
		PlayLevelSound(enterLevelSoundCue);
		isEnteringLevel = true;
		engineSound->Play();
		gameManager->PlayLevelSoundCue();
	}
}

void APlayerCarPawn::DoLevelExit()
{
	if (!isExitingLevel)
	{
		PlayLevelSound(exitLevelSoundCue);
		engineSound->Stop();
		gameManager->PlayPostLevelSoundCue();
		isExitingLevel = true;
		hasLevelStarted = false;
		thirdPersonCamera->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		
		thirdPersonCamera->SetActive(true);
		cameraEndLevelPosition = thirdPersonCamera->GetComponentLocation();
		cameraEndLevelRotation = thirdPersonCamera->GetComponentRotation();

		isLevelOver = true;
		UIWidget->ShowLevelOver();
		APlayerController* controller = GetWorld()->GetFirstPlayerController();
		controller->bShowMouseCursor = true;
		controller->bEnableClickEvents = true;
		controller->bEnableMouseOverEvents = true;
	}
}

#pragma endregion Level EnterExit

#pragma region Level Control

float APlayerCarPawn::GetDistanceRemaining()
{
	return distanceRemaining;
}

void APlayerCarPawn::Pause()
{
	if (isPaused)
	{
		UIWidget->HidePaused();
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
		isPaused = false;
		APlayerController* controller = GetWorld()->GetFirstPlayerController();
		controller->bShowMouseCursor = false;
		controller->bEnableClickEvents = false;
		controller->bEnableMouseOverEvents = false;
		gameManager->UnPauseMusic();
	}
	else
	{
		UIWidget->ShowPaused();
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.f);
		isPaused = true;
		APlayerController* controller = GetWorld()->GetFirstPlayerController();
		controller->bShowMouseCursor = true;
		controller->bEnableClickEvents = true;
		controller->bEnableMouseOverEvents = true;
		gameManager->PauseMusic();
	}
}

ULevel* APlayerCarPawn::GetCurrentLevelForRestart()
{
	return currentLevel;
}

#pragma endregion Level Control

//GEngine->AddOnScreenDebugMessage(99, 10, FColor::White, "TEST MESSAGE: " + FString("Test"));
//FScreenMessageString* testMessage = GEngine->ScreenMessages.Find(99);
//testMessage->TextScale.X = testMessage->TextScale.Y = 1.0f;