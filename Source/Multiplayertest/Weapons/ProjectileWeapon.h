// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponActor.h"
#include "ProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTEST_API AProjectileWeapon : public AWeaponActor
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;
	
private:

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass;

    UPROPERTY(EditAnywhere)
    TSubclassOf<AProjectile> ServerSideRewindProjectileClass;
};
