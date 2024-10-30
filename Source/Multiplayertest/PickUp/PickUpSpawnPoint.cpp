// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUpSpawnPoint.h"
#include "PickUp.h"

APickUpSpawnPoint::APickUpSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

}

void APickUpSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
    StartSpawnPickupTimer((AActor*)nullptr);
}

void APickUpSpawnPoint::SpawnPickup()
{
    int32 NumPickUpClasses = PickupClasses.Num();
    if (NumPickUpClasses > 0)
    {
        int32 Selection = FMath::RandRange(0, NumPickUpClasses - 1);
        SpawnedPickup = GetWorld()->SpawnActor<APickUp>(PickupClasses[Selection], GetActorTransform());
        

        if (HasAuthority() && SpawnedPickup)
        {
            SpawnedPickup->OnDestroyed.AddDynamic(this, &APickUpSpawnPoint::StartSpawnPickupTimer);
        }
    }
}

void APickUpSpawnPoint::SpawnPickupTimerFinished()
{
    if (HasAuthority())
    {
        SpawnPickup();
    }
}

void APickUpSpawnPoint::StartSpawnPickupTimer(AActor* DestroyedActor)
{
    const float SpawnTime = FMath::FRandRange(SpawnPickupTimeMin, SpawnPickupTimeMax);
    GetWorldTimerManager().SetTimer(
        SpawnPickupTimer,
        this,
        &APickUpSpawnPoint::SpawnPickupTimerFinished,
        SpawnTime
    );
}

void APickUpSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

