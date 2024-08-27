// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerAnimInstance.h"
#include "ShooterPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"


void UShooterPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ShooterPlayer = Cast<AShooterPlayer>(TryGetPawnOwner());
}

void UShooterPlayerAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (ShooterPlayer == nullptr)
	{
		ShooterPlayer = Cast<AShooterPlayer>(TryGetPawnOwner());
	}
	if (ShooterPlayer == nullptr) return;

	FVector Velocity = ShooterPlayer->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = ShooterPlayer->GetCharacterMovement()->IsFalling();
	bIsAccelerating = ShooterPlayer->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
}
