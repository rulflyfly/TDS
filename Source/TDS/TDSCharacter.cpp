// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "TDSCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Materials/Material.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "TDSGameInstance.h"
#include "Math/Vector.h"

ATDSCharacter::ATDSCharacter()
{
    // Set size for player capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    // Don't rotate character to camera direction
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
    GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
    GetCharacterMovement()->bConstrainToPlane = true;
    GetCharacterMovement()->bSnapToPlaneAtStart = true;

    // Create a camera boom...
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
    CameraBoom->TargetArmLength = 800.f;
    CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
    CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level
    
    // Create a camera...
    TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
    TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

    // Activate ticking in order to update the cursor every frame.
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
}

void ATDSCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (CurrentCursor)
    {
        APlayerController* myPC = Cast<APlayerController>(GetController());
        if (myPC)
        {
            FHitResult TraceHitResult;
            myPC->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);
            FVector CursorFV = TraceHitResult.ImpactNormal;
            FRotator CursorR = CursorFV.Rotation();

            CurrentCursor->SetWorldLocation(TraceHitResult.Location);
            CurrentCursor->SetWorldRotation(CursorR);
        }
    }

    MovementTick(DeltaSeconds);
}

void ATDSCharacter::BeginPlay()
{
    Super::BeginPlay();

    InitWeapon(InitWeaponName);

    if (CursorMaterial)
    {
        CurrentCursor = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), CursorMaterial, CursorSize, FVector(0));
    }
}

void ATDSCharacter::SetupPlayerInputComponent(UInputComponent* NewInputComponent)
{
    Super::SetupPlayerInputComponent(NewInputComponent);

    NewInputComponent->BindAxis(TEXT("MoveForward"), this, &ATDSCharacter::InputAxisX);
    NewInputComponent->BindAxis(TEXT("MoveRight"), this, &ATDSCharacter::InputAxisY);

    NewInputComponent->BindAction(TEXT("FireEvent"), EInputEvent::IE_Pressed, this, &ATDSCharacter::InputAttackPressed);
    NewInputComponent->BindAction(TEXT("FireEvent"), EInputEvent::IE_Released, this, &ATDSCharacter::InputAttackReleased);
    NewInputComponent->BindAction(TEXT("ReloadEvent"), EInputEvent::IE_Released, this, &ATDSCharacter::TryReloadWeapon);
}

void ATDSCharacter::InputAxisY(float Value)
{
    AxisY = Value;
}

void ATDSCharacter::InputAxisX(float Value)
{
    AxisX = Value;
}

void ATDSCharacter::InputAttackPressed()
{
    AttackCharEvent(true);
}

void ATDSCharacter::InputAttackReleased()
{
    AttackCharEvent(false);
}

void ATDSCharacter::MovementTick(float DeltaTime)
{
    AddMovementInput(FVector(1.0f, 0.0f, 0.0f), AxisX);
    AddMovementInput(FVector(0.0f, 1.0f, 0.0f), AxisY);

    FVector MovementDirection = GetLastMovementInputVector();
    FVector LookDirection = GetActorRotation().Vector();
    SprintAngle = UKismetMathLibrary::DegAcos(FVector::DotProduct(MovementDirection, LookDirection));
    
        APlayerController* myController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (myController)
        {
            FHitResult ResultHit;
            //myController->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery6, false, ResultHit);// bug was here Config\DefaultEngine.Ini
            myController->GetHitResultUnderCursor(ECC_GameTraceChannel1, true, ResultHit);

            float FindRotaterResultYaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), ResultHit.Location).Yaw;
            SetActorRotation(FQuat(FRotator(0.0f, FindRotaterResultYaw, 0.0f)));

            if (CurrentWeapon)
            {
                FVector Displacement = FVector(0);
                switch (MovementState)
                {
                case EMovementState::Aim_State:
                    Displacement = FVector(0.0f, 0.0f, 160.0f);
                    CurrentWeapon->ShouldReduceDispersion = true;
                    break;
                case EMovementState::AimWalk_State:
                    CurrentWeapon->ShouldReduceDispersion = true;
                    Displacement = FVector(0.0f, 0.0f, 160.0f);
                    break;
                case EMovementState::Walk_State:
                    Displacement = FVector(0.0f, 0.0f, 120.0f);
                    CurrentWeapon->ShouldReduceDispersion = false;
                    break;
                case EMovementState::Run_State:
                    Displacement = FVector(0.0f, 0.0f, 120.0f);
                    CurrentWeapon->ShouldReduceDispersion = false;
                    break;
                case EMovementState::SprintRun_State:
                    break;
                default:
                    break;
                }
                
                CurrentWeapon->ShootEndLocation = ResultHit.Location + Displacement;
                //aim cursor like 3d Widget?
            }
        }
    
}

void ATDSCharacter::AttackCharEvent(bool bIsFiring)
{
    
    AWeaponDefault* myWeapon = nullptr;
    myWeapon = GetCurrentWeapon();
    if (myWeapon)
    {
        //ToDo Check melee or range
        myWeapon->SetWeaponStateFire(bIsFiring);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ATDSCharacter::AttackCharEvent - CurrentWeapon -NULL"));
    }
}

void ATDSCharacter::CharacterUpdate()
{
    float ResSpeed = 600.0f;
    switch (MovementState)
    {
    case EMovementState::Aim_State:
        ResSpeed = MovementSpeedInfo.AimSpeedNormal;
        break;
    case EMovementState::AimWalk_State:
        ResSpeed = MovementSpeedInfo.AimSpeedWalk;
        break;
    case EMovementState::Walk_State:
        ResSpeed = MovementSpeedInfo.WalkSpeedNormal;
        break;
    case EMovementState::Run_State:
        ResSpeed = MovementSpeedInfo.RunSpeedNormal;
        break;
    case EMovementState::SprintRun_State:
        ResSpeed = MovementSpeedInfo.SprintRunSpeedRun;
        break;
    default:
        break;
    }

    GetCharacterMovement()->MaxWalkSpeed = ResSpeed;
}

void ATDSCharacter::ChangeMovementState()
{
    if (!WalkEnabled && !SprintRunEnabled && !AimEnabled)
    {
        MovementState = EMovementState::Run_State;
    }
    else
    {
        if (SprintRunEnabled)
        {
            WalkEnabled = false;
            AimEnabled = false;
            MovementState = EMovementState::SprintRun_State;
        }
        if (WalkEnabled && !SprintRunEnabled && AimEnabled)
        {
            MovementState = EMovementState::AimWalk_State;
        }
        else
        {
            if (WalkEnabled && !SprintRunEnabled && !AimEnabled)
            {
                MovementState = EMovementState::Walk_State;
            }
            else
            {
                if (!WalkEnabled && !SprintRunEnabled && AimEnabled)
                {
                    MovementState = EMovementState::Aim_State;
                }
            }
        }
    }
    CharacterUpdate();

    //Weapon state update
    AWeaponDefault* myWeapon = GetCurrentWeapon();
    if (myWeapon)
    {
        myWeapon->UpdateStateWeapon(MovementState);
    }
}

AWeaponDefault* ATDSCharacter::GetCurrentWeapon()
{
    return CurrentWeapon;
}

void ATDSCharacter::InitWeapon(FName IdWeaponName)
{
    
    UTDSGameInstance* myGI = Cast<UTDSGameInstance>(GetGameInstance());
    FWeaponInfo myWeaponInfo;
    if (myGI)
    {
        if (myGI->GetWeaponInfoByName(IdWeaponName, myWeaponInfo))
        {
            if (myWeaponInfo.WeaponClass)
            {
                FVector SpawnLocation = FVector(0);
                FRotator SpawnRotation = FRotator(0);

                FActorSpawnParameters SpawnParams;
                SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                SpawnParams.Owner = GetOwner();
                SpawnParams.Instigator = GetInstigator();

                AWeaponDefault* myWeapon = Cast<AWeaponDefault>(GetWorld()->SpawnActor(myWeaponInfo.WeaponClass, &SpawnLocation, &SpawnRotation, SpawnParams));
                if (myWeapon)
                {
                    FAttachmentTransformRules Rule(EAttachmentRule::SnapToTarget, false);
                    myWeapon->AttachToComponent(GetMesh(), Rule, FName("WeaponSocketRightHand"));
                    CurrentWeapon = myWeapon;
                    
                    myWeapon->WeaponSetting = myWeaponInfo;
                    myWeapon->WeaponInfo.Round = myWeaponInfo.MaxRound;
                    //Remove !!! Debug
                    myWeapon->ReloadTime = myWeaponInfo.ReloadTime;
                    myWeapon->UpdateStateWeapon(MovementState);

                    myWeapon->OnWeaponReloadStart.AddDynamic(this, &ATDSCharacter::WeaponReloadStart);
                    myWeapon->OnWeaponReloadEnd.AddDynamic(this, &ATDSCharacter::WeaponReloadEnd);
                }
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("ATDSCharacter::InitWeapon - Weapon not found in table -NULL"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No Game Instance :("));
    }

    
}

void ATDSCharacter::TryReloadWeapon()
{
    if (CurrentWeapon)
    {
        if (CurrentWeapon->GetWeaponRound() <= CurrentWeapon->WeaponSetting.MaxRound)
            CurrentWeapon->InitReload();
    }
}

void ATDSCharacter::WeaponReloadStart(UAnimMontage* Anim)
{
    WeaponReloadStart_BP(Anim);
}

void ATDSCharacter::WeaponReloadEnd()
{
    WeaponReloadEnd_BP();
}

void ATDSCharacter::WeaponReloadStart_BP_Implementation(UAnimMontage* Anim)
{
    // in BP
}

void ATDSCharacter::WeaponReloadEnd_BP_Implementation()
{
    // in BP
}

UDecalComponent* ATDSCharacter::GetCursorToWorld()
{
    return CurrentCursor;
}
