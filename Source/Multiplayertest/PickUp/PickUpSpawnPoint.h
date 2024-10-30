// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickUpSpawnPoint.generated.h"

UCLASS()
class MULTIPLAYERTEST_API APickUpSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:
    APickUpSpawnPoint();
    
    virtual void Tick(float DeltaTime) override;
protected:
    
    virtual void BeginPlay() override;
    
    UPROPERTY(EditAnywhere)
    TArray<TSubclassOf<class APickUp>> PickupClasses;
    
    UPROPERTY()
    APickUp* SpawnedPickup;
    
    void SpawnPickup();
    void SpawnPickupTimerFinished();
    
    UFUNCTION()
    void StartSpawnPickupTimer(AActor* DestroyedActor);

private:

    FTimerHandle SpawnPickupTimer;

    UPROPERTY(EditAnywhere)
    float SpawnPickupTimeMin;

    UPROPERTY(EditAnywhere)
    float SpawnPickupTimeMax;
public:

};
