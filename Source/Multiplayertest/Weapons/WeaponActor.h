#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Multiplayertest/Weapons/WeaponTypes.h"

#include "WeaponActor.generated.h"


UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
    EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMax")

};

UENUM(BlueprintType)
enum class EFireType : uint8
{
    EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
    EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
    EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),

    EFT_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class MULTIPLAYERTEST_API AWeaponActor : public AActor
{
	GENERATED_BODY()

public:
	AWeaponActor();

	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	void SetHUDAmmo();
	void ShowPickupWidget(bool bShowWidget);
	virtual void Fire(const FVector& HitTarget);

	void DroppedWeapon();

	void AddAmmo(int32 AmmoToAdd);

    FVector TraceEndWithScatter(const FVector& HitTarget);

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CrosshairsUp;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CrosshairsDown;

	//Zoom mientras apunta

	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;
	
	UPROPERTY(EditAnywhere)
	float ZoomedInterpSpeed= 20.f;

	//Modo de fuego automatico
	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = .15f;
	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true;

	UPROPERTY(EditAnywhere)
	class USoundCue* EquipSound;

	//Setear el custom depth

	void EnableCustomDepth(bool bEnable);

    bool bDestroyWeapon = false;

    UPROPERTY(EditAnywhere)
    EFireType FireType;

    UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
    bool bUseScatter = false;
protected:
	virtual void BeginPlay() override;

    virtual void OnWeaponStateSet();
    virtual void OnEquipped();
    virtual void OnDropped();
    virtual void OnEquippedSecondary();

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

    //Trace end con expansión

    UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
    float DistanceToSphere = 800.f;

    UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
    float SphereRadius = 75.f;


    UPROPERTY(EditAnywhere)
    float HeadShotDamage = 70.f;

    UPROPERTY(EditAnywhere)
    float Damage = 20.f;

    UPROPERTY(Replicated, EditAnywhere)
    bool bUseServerSideRewind = false;

    UPROPERTY()
    class AShooterPlayer* ShooterCharacter;

    UPROPERTY()
    class AShooterPlayerController* ShooterOwnerController;

    UFUNCTION()
    void OnPingTooHigh(bool bPingTooHigh);
private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere)
	int32 Ammo;

    UFUNCTION(Client, Reliable)
    void ClientUpdateAmmo(int32 ServerAmmo);

    UFUNCTION(Client, Reliable)
    void ClientAddAmmo(int32 AmmoToAdd);

	UPROPERTY(EditAnywhere)
	int32 MagCapacity;

    // Numero de request sin procesar del servidor para la munición.
    //Incrementado en SpendRound, Decrementado en ClientUpdateAmmo.
    int32 Sequence = 0;

	void SpendRound();

	

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;


public:
	FORCEINLINE void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomedInterpSpeed() const { return ZoomedInterpSpeed; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
    FORCEINLINE float GetDamage() const { return Damage; }
    FORCEINLINE float GetHeadShotDamage() const { return HeadShotDamage; }


	bool IsEmpty();
	bool IsFull();
};
