// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Multiplayertest/Interfaces/InteractCrosshairsInterface.h"
#include "Multiplayertest/BlasterTypes/TurnInPlace.h"
#include "ShooterPlayer.generated.h"


UCLASS()
class MULTIPLAYERTEST_API AShooterPlayer : public ACharacter, public IInteractCrosshairsInterface
{
	GENERATED_BODY()

public:
	AShooterPlayer();
	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

	void PlayShootingMontage(bool bAiming);

protected:
	
	virtual void BeginPlay() override;

	void Forward(float value);
	void Right(float value);
	void Turn(float value);
	void Yaw(float value);
	
	virtual void Jump() override;

	void EquippedPressedButton();
	void CrouchButtonPressed();
	
	void AimButtonPressed();
	void AimButtonReleased();

	void AimOffset(float DeltaTime);

	void FireButtonPressed();
	void FireButtonReleased();


private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	/*UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverHeadWidget; */

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeaponActor* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeaponActor* LastWeapon);

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* CombatComp;

	UFUNCTION(Server, Reliable)
	void ServerEquippedButtonPressed();

	float AO_Yaw;
	float InterptAO_Way;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	UPROPERTY(EditAnywhere, Category= Combat)
	class UAnimMontage* FireWeaponMontage;


public:
	void SetOverlappingWeapon(AWeaponActor* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();

	FVector GetHitTarget() const;
	AWeaponActor* GetEquippedWeapon();

	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
