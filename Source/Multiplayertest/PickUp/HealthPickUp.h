// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickUp.h"
#include "HealthPickUp.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTEST_API AHealthPickUp : public APickUp
{
	GENERATED_BODY()
	
public: 
    AHealthPickUp();
protected:

    virtual void OnSphereOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

private:

    UPROPERTY(EditAnywhere)
    float HealAmount = 100.f;

    UPROPERTY(EditAnywhere)
    float HealingTime = 5.f;

};
