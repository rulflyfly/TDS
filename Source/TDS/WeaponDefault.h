// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"

#include "Types.h"
#include "ProjectileDefault.h"
#include "WeaponDefault.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponFireStart);//ToDo Delegate on event weapon fire - Anim char, state char...
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponReloadStart,UAnimMontage*,Anim);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponReloadEnd);

UCLASS()
class TDS_API AWeaponDefault : public AActor
{
    GENERATED_BODY()
    
public:
    // Sets default values for this actor's properties
    AWeaponDefault();

    FOnWeaponReloadEnd OnWeaponReloadEnd;
    FOnWeaponReloadStart OnWeaponReloadStart;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
    class USceneComponent* SceneComponent = nullptr;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
    class USkeletalMeshComponent* SkeletalMeshWeapon = nullptr;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
    class UStaticMeshComponent* StaticMeshWeapon = nullptr;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
    class UArrowComponent* ShootLocation = nullptr;

    UPROPERTY()
    FWeaponInfo WeaponSetting;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Info")
    FAddicionalWeaponInfo WeaponInfo;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Tick func
    virtual void Tick(float DeltaTime) override;

    void FireTick(float DeltaTime);
    void ReloadTick(float DeltaTime);
    void DispersionTick(float DeltaTime);

    void WeaponInit();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FireLogic")
    bool WeaponFiring = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReloadLogic")
    bool WeaponReloading = false;

    UFUNCTION(BlueprintCallable)
    void SetWeaponStateFire(bool bIsFire);

    bool CheckWeaponCanFire();

    FProjectileInfo GetProjectile();

    void Fire();

    void UpdateStateWeapon(EMovementState NewMovementState);
    void ChangeDispersionByShot();
    float GetCurrentDispersion() const;
    FVector ApplyDispersionToShoot(FVector DirectionShoot)const;

    FVector GetFireEndLocation()const;
    int8 GetNumberProjectileByShot() const;

    //Timers
    float FireTimer = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReloadLogic")
    float ReloadTimer = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReloadLogic Debug")    //Remove !!! Debug
    float ReloadTime = 0.0f;
    
    //flags
    bool BlockFire = false;
    //Dispersion
    bool ShouldReduceDispersion = false;
    float CurrentDispersion = 0.0f;
    float CurrentDispersionMax = 1.0f;
    float CurrentDispersionMin = 0.1f;
    float CurrentDispersionRecoil = 0.1f;
    float CurrentDispersionReduction = 0.1f;

    FVector ShootEndLocation = FVector(0);

    UFUNCTION(BlueprintCallable)
    int32 GetWeaponRound();
    void InitReload();
    void FinishReload();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
        bool ShowDebug = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
        float SizeVectorToChangeShootDirectionLogic = 100.0f;
};
