// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Multiplayertest/Interfaces/InteractCrosshairsInterface.h"
#include "Multiplayertest/BlasterTypes/TurnInPlace.h"
#include "Multiplayertest/PlayerState/ShooterPlayerState.h"
#include "Multiplayertest/BlasterTypes/CombatState.h"
#include "Multiplayertest/Weapons/Projectile.h"

#include "ShooterPlayer.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

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

    //Animation Montages del personaje
    void PlayShootingMontage(bool bAiming);
    void PlayEliminationMontage();
    void PlayReloadMontage();
    void PlayThrowGrenadeMontage();
    void PlaySwapWeapon();

    //Metodos del HUD
    void UpdateHUDHealth();
    void UpdateHUDShield();
    void UpdateHUDAmmo();

    void SpawnDefaultWeapon();

    UPROPERTY()
    TMap<FName, class UBoxComponent*> HitCollisionBoxes;

    bool bFinishedSwapping = false;

    virtual void OnRep_ReplicatedMovement() override;

    void Elim(bool bPlayerLeftGame);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastElim(bool bPlayerLeftGame);

    virtual void Destroyed() override;

    UPROPERTY(Replicated)
    bool bDisableGameplay = false;

    UFUNCTION(BlueprintImplementableEvent)
    void ShowSniperScopeWidget(bool bShowScope);

    UFUNCTION(Server, Reliable)
    void ServerLeaveGame();

    FOnLeftGame OnLeftGame;

    UFUNCTION(NetMulticast, Reliable)
    void MulticastGainedTheLead();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastLostTheLead();

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

    void DropOrDestroyWeapon(AWeaponActor* Weapon);
    void DropOrDestroyWeapons();

    UFUNCTION()
    void ReceiveDamage(AActor* DamagedACtor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCausor);

    void PollInit();
    void RotateInPlace(float DeltaTime);


    //HitBoxes para el Server Side rewind

    UPROPERTY(EditAnywhere)
    class UBoxComponent* head;

    UPROPERTY(EditAnywhere)
    UBoxComponent* pelvis;

    UPROPERTY(EditAnywhere)
    UBoxComponent* spine_02;

    UPROPERTY(EditAnywhere)
    UBoxComponent* spine_03;

    UPROPERTY(EditAnywhere)
    UBoxComponent* upperarm_l;

    UPROPERTY(EditAnywhere)
    UBoxComponent* upperarm_r;

    UPROPERTY(EditAnywhere)
    UBoxComponent* lowerarm_l;

    UPROPERTY(EditAnywhere)
    UBoxComponent* lowerarm_r;

    UPROPERTY(EditAnywhere)
    UBoxComponent* hand_l;

    UPROPERTY(EditAnywhere)
    UBoxComponent* hand_r;

    UPROPERTY(EditAnywhere)
    UBoxComponent* backpack;

    UPROPERTY(EditAnywhere)
    UBoxComponent* blanket;

    UPROPERTY(EditAnywhere)
    UBoxComponent* thigh_l;

    UPROPERTY(EditAnywhere)
    UBoxComponent* thigh_r;

    UPROPERTY(EditAnywhere)
    UBoxComponent* calf_l;

    UPROPERTY(EditAnywhere)
    UBoxComponent* calf_r;

    UPROPERTY(EditAnywhere)
    UBoxComponent* foot_l;

    UPROPERTY(EditAnywhere)
    UBoxComponent* foot_r;

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

    //ShooterPlayer components

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    class UCombatComponent* CombatComp;

    UPROPERTY(VisibleAnywhere)
    class ULagCompensationComponent* LagCompensationComp;

    UPROPERTY(VisibleAnywhere)
    class UBuffComponent* BuffComp;

    UFUNCTION(Server, Reliable)
    void ServerEquippedButtonPressed();

    float AO_Yaw;
    float InterptAO_Way;
    float AO_Pitch;
    FRotator StartingAimRotation;

    ETurningInPlace TurningInPlace;
    void TurnInPlace(float DeltaTime);

    UPROPERTY(EditAnywhere, Category = Combat)
    class UAnimMontage* FireWeaponMontage;

    UPROPERTY(EditAnywhere, Category = Combat)
    class UAnimMontage* HitReactMontage;

    UPROPERTY(EditAnywhere, Category = Combat)
    class UAnimMontage* EliminationMontage;

    UPROPERTY(EditAnywhere, Category = Combat)
    class UAnimMontage* ReloadMontage;

    UPROPERTY(EditAnywhere, Category = Combat)
    UAnimMontage* ThrowGrenadeMontage;

    UPROPERTY(EditAnywhere, Category = Combat)
    UAnimMontage* WeaponSwapMontage;

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

    UPROPERTY(EditAnywhere, Category = "Player Stats")
    float MaxHealth = 100.f;

    UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "PlayerStats")
    float CurrHealth = 100.f;

    bool bIsEliminated = false;

    FTimerHandle ElimTimer;

    UPROPERTY(EditDefaultsOnly)
    float ElimDelay = 3.f;

    void ElimTimerFinished();

    bool bLeftGame = false;

    UFUNCTION()
    void OnRep_Health(float LastHealth);

    //Player Shield

    UPROPERTY(EditAnywhere, Category = "Player Stats")
    float MaxShield = 100.f;

    UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = "Player Stats")
    float CurrShield = 25.f;

    UFUNCTION()
    void OnRep_Shield(float LastShield);

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

    UPROPERTY(EditAnywhere)
    class UNiagaraSystem* CrownSystem;

    UPROPERTY()
    class UNiagaraComponent* CrownComponent;

    //Granada
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* AttachedGrenade;

    //Arma por defecto

    UPROPERTY(EditAnywhere)
    TSubclassOf<AWeaponActor> DefaultWeapon;

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

    FORCEINLINE void SetHealth(float Amount) { CurrHealth = Amount; }
    FORCEINLINE float GetCurrHealth() const { return CurrHealth; }
    FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
    
    FORCEINLINE void SetShield(float Amount) { CurrShield = Amount; }
    FORCEINLINE float GetShield() const { return CurrShield; }
    FORCEINLINE float GetMaxShield() const { return MaxShield; }

    FORCEINLINE UCombatComponent* GetCombat() const { return CombatComp; }
    FORCEINLINE ULagCompensationComponent* GetLagCompensation() const { return LagCompensationComp; }
    FORCEINLINE UBuffComponent* GetBuff() const { return BuffComp; }
    FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
    FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
    FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
    
    bool IsLocallyReloading();
    ECombatState GetCombatState() const;
};
