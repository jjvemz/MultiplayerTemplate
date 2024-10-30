// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickUp.h"
#include "Multiplayertest/Weapons/WeaponTypes.h"
#include "AmmoPickUp.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTEST_API AAmmoPickUp : public APickUp
{
	GENERATED_BODY()

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
    int32 AmmoAmount = 30;

    UPROPERTY(EditAnywhere)
    EWeaponType WeaponType;
};
