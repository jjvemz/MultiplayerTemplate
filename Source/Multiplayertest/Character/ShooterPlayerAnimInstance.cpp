// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerAnimInstance.h"
#include "ShooterPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"


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
	bWeaponEquipped = ShooterPlayer->IsWeaponEquipped();
	bIsCrouched = ShooterPlayer->bIsCrouched;
	bAiming = ShooterPlayer->IsAiming();

	FRotator AimRotation = ShooterPlayer->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterPlayer->GetVelocity());
	YawOffset = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = ShooterPlayer->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);
}
