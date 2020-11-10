// Copyright Epic Games, Inc. All Rights Reserved.

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
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
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

	// Create a decal in the world to show the cursor's location
	CursorToWorld = CreateDefaultSubobject<UDecalComponent>("CursorToWorld");
	CursorToWorld->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UMaterial>
    DecalMaterialAsset(TEXT("Material'/Game/Bluprint/Character/M_Cursor_Decal.M_Cursor_Decal'"));
	if (DecalMaterialAsset.Succeeded())
	{
		CursorToWorld->SetDecalMaterial(DecalMaterialAsset.Object);
	}
	CursorToWorld->DecalSize = FVector(16.0f, 32.0f, 32.0f);
	CursorToWorld->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ATDSCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

	if (CursorToWorld != nullptr)
	{
		if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
		{
			if (UWorld* World = GetWorld())
			{
				FHitResult HitResult;
				FCollisionQueryParams Params(NAME_None, FCollisionQueryParams::GetUnknownStatId());
				FVector StartLocation = TopDownCameraComponent->GetComponentLocation();
				FVector EndLocation = TopDownCameraComponent->GetComponentRotation().Vector() * 2000.0f;
				Params.AddIgnoredActor(this);
				World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, Params);
				FQuat SurfaceRotation = HitResult.ImpactNormal.ToOrientationRotator().Quaternion();
				CursorToWorld->SetWorldLocationAndRotation(HitResult.Location, SurfaceRotation);
			}
		}
		else if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			FHitResult TraceHitResult;
			PC->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);
			FVector CursorFV = TraceHitResult.ImpactNormal;
			FRotator CursorR = CursorFV.Rotation();
			CursorToWorld->SetWorldLocation(TraceHitResult.Location);
			CursorToWorld->SetWorldRotation(CursorR);
		}
	}
    
    MovementTick(DeltaSeconds);
    

}

void ATDSCharacter::SetupPlayerInputComponent(class UInputComponent* NewInputComponent)
{
    Super::SetupPlayerInputComponent(InputComponent);
    NewInputComponent->BindAxis(TEXT("MoveForward"), this, &ATDSCharacter::InputAxisX);
    NewInputComponent->BindAxis(TEXT("MoveRight"), this, &ATDSCharacter::InputAxisY);
}

void ATDSCharacter::InputAxisX(float Value)
{
    AxisX = Value;
}

void ATDSCharacter::InputAxisY(float Value)
{
    AxisY = Value;
}

void ATDSCharacter::MovementTick(float DeltaTime)
{
    AddMovementInput(FVector(1.f, 0.f, 0.f), AxisX);
    AddMovementInput(FVector(0.f, 1.f, 0.f), AxisY);
    
//    if (SprintRunEnabled)
//    {
//        AddMovementInput(GetActorForwardVector(), 1);
//    }
//
    
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PlayerController)
    {
        FHitResult HitResult;
        PlayerController->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery6, false, HitResult);
        auto FindRotatorResultYaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), HitResult.Location).Yaw;
        SetActorRotation(FQuat(FRotator(0.f, FindRotatorResultYaw, 0.f)));
    }
    
 

    
    FVector MovementDirection = GetLastMovementInputVector();
    FVector LookDirection = GetActorRotation().Vector();
    SprintAngle = UKismetMathLibrary::DegAcos(FVector::DotProduct(MovementDirection, LookDirection));

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
}
