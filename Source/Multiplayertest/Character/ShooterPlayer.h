// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Multiplayertest/Interfaces/InteractCrosshairsInterface.h"
#include "Multiplayertest/BlasterTypes/TurnInPlace.h"
#include "Multiplayertest/PlayerState/ShooterPlayerState.h"
#include "Multiplayertest/BlasterTypes/CombatState.h"

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
	void PlayEliminationMontage();
	void PlayReloadMontage();
	void PlayThrowGrenadeMontage();

	virtual void OnRep_ReplicatedMovement() override;
	
	void Elim();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	virtual void Destroyed() override;

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;
	
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

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
	void CalculateAO_Pitch();
	void SimProxiesTurn();

	void FireButtonPressed();
	void FireButtonReleased();

	void ReloadButtonPressed();
	void GrenadeButtonPressed();

	void PlayHitReactMontage();

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedACtor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCausor);
	void UpdateHUDHealth();

	void PollInit();
	void RotateInPlace(float DeltaTime);
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
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

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* EliminationMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ThrowGrenadeMontage;

	void HideCameraIfTheCaharacterisClose();

	UPROPERTY(EditAnywhere, Category = CameraTweaks)
	float CameraThreshold = 200.f;

	bool bRotateRootBone;

	UPROPERTY(EditAnywhere, Category = CameraTweaks)

	float TurnThreshold = 0.5f;

	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;

	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CalculateSpeed();

	//Vida del jugador

	UPROPERTY(EditAnywhere, Category="Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing= OnRep_Health, VisibleAnywhere, Category="PlayerStats")
	float CurrHealth = 100.f;

	bool bIsEliminated = false;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	void ElimTimerFinished();

	UFUNCTION()
	void OnRep_Health();

	UPROPERTY()
	class AShooterPlayerController* ShooterPlayerController;

	//Elim Bot
	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;
	
	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;
	
	UPROPERTY(EditAnywhere)
	class USoundCue* ElimBotSound;

	UPROPERTY()
	class AShooterPlayerState* ShooterPlayerState;

    //Granada

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* AttachedGrenade;

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
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsEliminated() const { return bIsEliminated; }
	FORCEINLINE float GetCurrHealth() const { return CurrHealth; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE UCombatComponent* GetCombat() const { return CombatComp; }
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
    FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }

	ECombatState GetCombatState() const;
};
