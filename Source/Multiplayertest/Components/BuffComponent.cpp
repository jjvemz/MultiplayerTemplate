// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Multiplayertest/Character/ShooterPlayer.h"

UBuffComponent::UBuffComponent()
{
	
	PrimaryComponentTick.bCanEverTick = true;

}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
    bIsHealing = true;
    HealingRate = HealAmount / HealingTime;
    AmountToHeal += HealAmount;
}

void UBuffComponent::ReplenishShield(float ShieldAmount, float ReplenishTime)
{
    bReplenishingShield = true;
    ShieldReplenishRate = ShieldAmount / ReplenishTime;
    ShieldReplenishAmount += ShieldAmount;
}

void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
    if (Character == nullptr) return;
    Character->GetWorldTimerManager().SetTimer(
        SpeedBuffTimer,
        this,
        &UBuffComponent::ResetSpeeds,
        BuffTime
    );
    if (Character->GetCharacterMovement())
    {
        Character->GetCharacterMovement()->MaxWalkSpeed = BuffBaseSpeed;
        Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffCrouchSpeed;
    }
    MulticastSpeedBuff(BuffBaseSpeed, BuffCrouchSpeed);

}

void UBuffComponent::SetInitialSpeeds(float BaseSpeed, float CrouchSpeed)
{
    InitialBaseSpeed = BaseSpeed;
    InitialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffTime)
{
    if (Character == nullptr) return;

    Character->GetWorldTimerManager().SetTimer(
        JumpBuffTimer,
        this,
        &UBuffComponent::ResetJump,
        BuffTime
    );

    if (Character->GetCharacterMovement())
    {
        Character->GetCharacterMovement()->JumpZVelocity = BuffJumpVelocity;
    }
    MulticastJumpBuff(BuffJumpVelocity);

}

void UBuffComponent::SetInitialJumpVelocity(float Velocity)
{
    InitialJumpVelocity = Velocity;
}

void UBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity)
{
    if (Character && Character->GetCharacterMovement())
    {
        Character->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
    }
}

void UBuffComponent::ResetSpeeds()
{
    if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
    Character->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;
    Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
    MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed);
}

void UBuffComponent::ResetJump()
{
    if (Character->GetCharacterMovement())
    {
        Character->GetCharacterMovement()->JumpZVelocity = InitialJumpVelocity;
    }
    MulticastJumpBuff(InitialJumpVelocity);
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
    Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
    Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
}



void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

void UBuffComponent::HealthRampUp(float DeltaTime)
{

    if (!bIsHealing || Character == nullptr || Character->IsEliminated()) return;

    const float HealThisFrame = HealingRate * DeltaTime;
    Character->SetHealth(FMath::Clamp(Character->GetCurrHealth() + HealThisFrame, 0.f, Character->GetMaxHealth()));
    Character->UpdateHUDHealth();
    AmountToHeal -= HealThisFrame;

    if (AmountToHeal <= 0.f || Character->GetCurrHealth() >= Character->GetMaxHealth())
    {
        bIsHealing = false;
        AmountToHeal = 0.f;
    }
}

void UBuffComponent::ShieldRampUp(float DeltaTime)
{
    if (!bReplenishingShield || Character == nullptr || Character->IsEliminated()) return;

    const float ReplenishThisFrame = ShieldReplenishRate * DeltaTime;
    Character->SetShield(FMath::Clamp(Character->GetShield() + ReplenishThisFrame, 0.f, Character->GetMaxShield()));
    Character->UpdateHUDShield();
    ShieldReplenishAmount -= ReplenishThisFrame;

    if (ShieldReplenishAmount <= 0.f || Character->GetShield() >= Character->GetMaxShield())
    {
        bReplenishingShield = false;
        ShieldReplenishAmount = 0.f;
    }
}


void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    HealthRampUp(DeltaTime);
    ShieldRampUp(DeltaTime);
}

