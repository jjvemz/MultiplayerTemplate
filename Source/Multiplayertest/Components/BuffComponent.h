// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERTEST_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
    friend class AShooterPlayer;
    void Heal(float HealAmount, float HealingTime);

    void ReplenishShield(float ShieldAmount, float ReplenishTime);

    void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
    void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);

    void BuffJump(float BuffJumpVelocity, float BuffTime);
    void SetInitialJumpVelocity(float Velocity);

protected:
	virtual void BeginPlay() override;

    void HealthRampUp(float DeltaTime);
    void ShieldRampUp(float DeltaTime);


private:
    UPROPERTY()
    class AShooterPlayer* Character;

    //Speed Buff

    FTimerHandle SpeedBuffTimer;
    void ResetSpeeds();
    float InitialBaseSpeed;
    float InitialCrouchSpeed;

    UFUNCTION(NetMulticast, Reliable)
    void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);


    //Jump Buff

    FTimerHandle JumpBuffTimer;
    void ResetJump();

    float InitialJumpVelocity;

    UFUNCTION(NetMulticast, Reliable)
    void MulticastJumpBuff(float JumpVelocity);

    //Healing Buff

    bool bIsHealing = false;
    float HealingRate = 0.f;
    float AmountToHeal = 0.f;

    //Shield Buff

    bool bReplenishingShield = false;
    float ShieldReplenishRate = 0.f;
    float ShieldReplenishAmount = 0.f;


public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
