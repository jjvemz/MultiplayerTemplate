// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerAnimInstance.h"
#include "ShooterPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Multiplayertest/Weapons/WeaponActor.h"
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
	EquippedWeapon = ShooterPlayer->GetEquippedWeapon();
	bIsCrouched = ShooterPlayer->bIsCrouched;
	bAiming = ShooterPlayer->IsAiming();
	TurningInPlace = ShooterPlayer->GetTurningInPlace();
	bRotateRootBone = ShooterPlayer->ShouldRotateRootBone();
	bIsEliminated = ShooterPlayer->IsEliminated();

	FRotator AimRotation = ShooterPlayer->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterPlayer->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = ShooterPlayer->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = ShooterPlayer->GetAO_Yaw();
	AO_Pitch = ShooterPlayer->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && ShooterPlayer->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		ShooterPlayer->GetMesh()->TransformToBoneSpace(FName("Hand_R"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (ShooterPlayer->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
			RightHandRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - ShooterPlayer->GetHitTarget()));
		}

		FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
	}
}