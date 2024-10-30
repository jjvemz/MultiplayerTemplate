// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickUp.generated.h"

UCLASS()
class MULTIPLAYERTEST_API APickUp : public AActor
{
	GENERATED_BODY()
	
public:	
	APickUp();
    virtual void Tick(float DeltaTime) override;
    virtual void Destroyed() override;


    UFUNCTION()
    virtual void OnSphereOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

    UPROPERTY(EditAnywhere)
    float TurnRate = 45.f;

protected:
	virtual void BeginPlay() override;

private:	
	
    UPROPERTY(EditAnywhere)
	class USphereComponent* OverlapSphere;

    UPROPERTY(EditAnywhere)
    class USoundCue* PickupSound;

    UPROPERTY(EditAnywhere)
    UStaticMeshComponent* PickupMesh;

    UPROPERTY(VisibleAnywhere)
    class UNiagaraComponent* PickupEffectComponent;

    UPROPERTY(EditAnywhere)
    class UNiagaraSystem* PickupEffect;

    FTimerHandle BindOverlapTimer;
    float BindOverlapTime = 0.25f;
    void BindOverlapTimerFinished();

public:
};
