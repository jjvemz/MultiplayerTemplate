// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Multiplayertest/HUD/ShooterHUD.h"
#include "Multiplayertest/Weapons/WeaponTypes.h"
#include "Multiplayertest/BlasterTypes/CombatState.h"
#include "CombatComponent.generated.h"

//#define TRACE_LENGTH 12000.f;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERTEST_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class AShooterPlayer;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	

	void EquipWeapon(class AWeaponActor* WeaponToEquip);

	void Reload();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	void FireButtonPressed(bool bPressed);

	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	void JumpToShotgunEnd();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

    UFUNCTION(BlueprintCallable)
    void LaunchGrenade();

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();

	int32 AmountToReload();

	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	UFUNCTION()
	void OnRep_EquippedWeapon();


	void Fire();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshair(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);


    void DropEquippedWeapon();
    
    void AttachActorToRightHand(AActor* ActorToAttach);
    
    void AttachActorToLeftHand(AActor* ActorToAttach);
    
    void UpdateCarriedAmmo();

    void PlayEquipWeaponSound();

    void ReloadEmptyWeapon();

    void ShowAttachedGrenade(bool bShowGrenade);
private:
	UPROPERTY()
	class AShooterPlayer* Character;

	UPROPERTY()
	class AShooterPlayerController* Controller;
	
	UPROPERTY()
	class AShooterHUD* HUD;

	
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	class AWeaponActor* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming = false;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFirePressed;

	FVector HitTarget;

	/*
		Para el HUD y la mira
	*/

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairCrouchedFactor;
	float CrosshairShootingFactor;

	UPROPERTY(EditAnywhere, Category = Combat)
	float InterpToCrossHairAimFactor = -0.58f;

	FHUDPackage HUDPackage;

	//Apuntado y FOV

	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category= Combat)
	float ZoomedFOV = 30.f;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	/*Disparo automatico*/

	FTimerHandle FireTimer;
	bool bCanFire = true;

	void StartFireTimer();
	void FireTimerFinished();
	bool CanFire();

	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap <EWeaponType, int32> CarriedAmmoMap;

	//Ammo types
	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;

	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 0;
	
	UPROPERTY(EditAnywhere)
	int32 StartingSniperRifleAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo = 0;

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValues();

	void UpdateShotgunAmmoValues();

	void InitializeCarriedAmmo();
public:	


		
};
