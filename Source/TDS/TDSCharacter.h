// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Types.h"
#include "WeaponDefault.h"
//#include "Components/WidgetComponent.h"

#include "TDSCharacter.generated.h"

//USTRUCT(BlueprintType)
//struct FCharacterSpeedInfo
//{
//    GENERATED_BODY()
//
//
//};
UCLASS(Blueprintable)
class ATDSCharacter : public ACharacter
{
    GENERATED_BODY()
protected:
    virtual void BeginPlay() override;

public:
    ATDSCharacter();

    // Called every frame.
    virtual void Tick(float DeltaSeconds) override;

    virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

    /** Returns TopDownCameraComponent subobject **/
    FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
    /** Returns CameraBoom subobject **/
    FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    /** Returns CursorToWorld subobject **/
    //FORCEINLINE class UDecalComponent* GetCursorToWorld() { return CursorToWorld; }

private:
    /** Top down camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* TopDownCameraComponent;

    /** Camera boom positioning the camera above the character */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* CameraBoom;

    /** A decal that projects to the cursor location. */
    //UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    //class UDecalComponent* CursorToWorld;

public:
    //Cursor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
        UMaterialInterface* CursorMaterial = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
        FVector CursorSize = FVector(20.0f, 40.0f, 40.0f);


    //Movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
        EMovementState MovementState = EMovementState::Run_State;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
        FCharacterSpeed MovementSpeedInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
        bool SprintRunEnabled = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
        bool WalkEnabled = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
        bool AimEnabled = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float SprintAngle = 0.f;

    //Weapon
    AWeaponDefault* CurrentWeapon = nullptr;

    //for demo
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demo")
    FName InitWeaponName;

    UDecalComponent* CurrentCursor = nullptr;

    //Inputs
    UFUNCTION()
        void InputAxisY(float Value);
    UFUNCTION()
        void InputAxisX(float Value);
    UFUNCTION()
        void InputAttackPressed();
    UFUNCTION()
        void InputAttackReleased();

    float AxisX = 0.0f;
    float AxisY = 0.0f;

    // Tick Func
    UFUNCTION()
    void MovementTick(float DeltaTime);

    //Func
    UFUNCTION(BlueprintCallable)
        void AttackCharEvent(bool bIsFiring);
    UFUNCTION(BlueprintCallable)
        void CharacterUpdate();
    UFUNCTION(BlueprintCallable)
        void ChangeMovementState();

    UFUNCTION(BlueprintCallable)
        AWeaponDefault* GetCurrentWeapon();
    UFUNCTION(BlueprintCallable)
        void InitWeapon(FName IdWeaponName);
    UFUNCTION(BlueprintCallable)
    void TryReloadWeapon();
    UFUNCTION()
        void WeaponReloadStart(UAnimMontage* Anim);
    UFUNCTION()
        void WeaponReloadEnd();
    UFUNCTION(BlueprintNativeEvent)
        void WeaponReloadStart_BP(UAnimMontage* Anim);
    UFUNCTION(BlueprintNativeEvent)
        void WeaponReloadEnd_BP();

    UFUNCTION(BlueprintCallable)
    UDecalComponent* GetCursorToWorld();
};

