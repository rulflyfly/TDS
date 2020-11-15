// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileDefault.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AProjectileDefault::AProjectileDefault()
{
     // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    BulletCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Sphere"));

    BulletCollisionSphere->SetSphereRadius(16.f);

    BulletCollisionSphere->bReturnMaterialOnMove = true;//hit event return physMaterial

    BulletCollisionSphere->SetCanEverAffectNavigation(false);//collision not affect navigation (P keybord on editor)

    RootComponent = BulletCollisionSphere;

    BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bullet Projectile Mesh"));
    BulletMesh->SetupAttachment(RootComponent);
    BulletMesh->SetCanEverAffectNavigation(false);

    BulletFX = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Bullet FX"));
    BulletFX->SetupAttachment(RootComponent);

    //BulletSound = CreateDefaultSubobject<UAudioComponent>(TEXT("Bullet Audio"));
    //BulletSound->SetupAttachment(RootComponent);

    BulletProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Bullet ProjectileMovement"));
    BulletProjectileMovement->UpdatedComponent = RootComponent;
    BulletProjectileMovement->InitialSpeed = 1.f;
    BulletProjectileMovement->MaxSpeed = 0.f;

    BulletProjectileMovement->bRotationFollowsVelocity = true;
    BulletProjectileMovement->bShouldBounce = true;
}

// Called when the game starts or when spawned
void AProjectileDefault::BeginPlay()
{
    Super::BeginPlay();

    BulletCollisionSphere->OnComponentHit.AddDynamic(this, &AProjectileDefault::BulletCollisionSphereHit);
    BulletCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AProjectileDefault::BulletCollisionSphereBeginOverlap);
    BulletCollisionSphere->OnComponentEndOverlap.AddDynamic(this, &AProjectileDefault::BulletCollisionSphereEndOverlap);
}

// Called every frame
void AProjectileDefault::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

void AProjectileDefault::InitProjectile(FProjectileInfo InitParam)
{
    BulletProjectileMovement->InitialSpeed = InitParam.ProjectileInitSpeed;
    BulletProjectileMovement->MaxSpeed = InitParam.ProjectileInitSpeed;
    this->SetLifeSpan(InitParam.ProjectileLifeTime);

    ProjectileSetting = InitParam;
}

void AProjectileDefault::BulletCollisionSphereHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (OtherActor && Hit.PhysMaterial.IsValid())
    {
        EPhysicalSurface mySurfacetype = UGameplayStatics::GetSurfaceType(Hit);

        if (ProjectileSetting.HitDecals.Contains(mySurfacetype))
        {
            UMaterialInterface* myMaterial = ProjectileSetting.HitDecals[mySurfacetype];

            if (myMaterial && OtherComp)
            {
                UGameplayStatics::SpawnDecalAttached(myMaterial, FVector(20.0f), OtherComp, NAME_None, Hit.ImpactPoint, Hit.ImpactNormal.Rotation(),EAttachLocation::KeepWorldPosition,10.0f);
            }
        }
        if (ProjectileSetting.HitFXs.Contains(mySurfacetype))
        {
            UParticleSystem* myParticle = ProjectileSetting.HitFXs[mySurfacetype];
            if (myParticle)
            {
                UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), myParticle, FTransform(Hit.ImpactNormal.Rotation(), Hit.ImpactPoint, FVector(1.0f)));
            }
        }
            
        if (ProjectileSetting.HitSound)
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), ProjectileSetting.HitSound, Hit.ImpactPoint);
        }
    
    }
    UGameplayStatics::ApplyDamage(OtherActor, ProjectileSetting.ProjectileDamage, GetInstigatorController(), this, NULL);
    ImpactProjectile();
    //UGameplayStatics::ApplyRadialDamageWithFalloff()
    //Apply damage cast to if char like bp? //OnAnyTakeDmage delegate
    //UGameplayStatics::ApplyDamage(OtherActor, ProjectileSetting.ProjectileDamage, GetOwner()->GetInstigatorController(), GetOwner(), NULL);
    //or custom damage by health component
    
}

void AProjectileDefault::BulletCollisionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void AProjectileDefault::BulletCollisionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void AProjectileDefault::ImpactProjectile()
{
    this->Destroy();
}


