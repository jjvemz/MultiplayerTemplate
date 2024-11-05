// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);
/**
 * 
 */
UCLASS()
class MULTIPLAYERTEST_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:


	void SetHUDHealth(float CurrHealth, float MaxHealth);
    void SetHUDShield(float CurrShield, float MaxShield);
	virtual void OnPossess(APawn* InPawn) override;
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);

	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDcarriedAmmo(int32 Ammo);
    void SetHUDGrenades(int32 Grenades);

	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);

	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual float GetServerTime();//Se Sincroniza con el world clock del servidor
	virtual void ReceivedPlayer() override; // Se sincroniza con el reloj del servidor lo antes posible

	void OnMatchStateSet(FName State);
	//Metodos para cada estado de la partida
	void HandleMatchHasStarted();
	void HandleCooldown();

    float SingleTripTime = 0;

    FHighPingDelegate HighPingDelegate;
protected:

	virtual void BeginPlay() override;

    virtual void SetupInputComponent() override;

	void SetHUDTime();
	void PollInit();

	//Solicita el tiempo actual del servidor.
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	UFUNCTION(Server, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f;

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);
	
	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime);

    void HighPingWarning();
    void StopHighPingWarning();
    void CheckPing(float DeltaTime);

    void ShowReturnToMainMenu();

private:

    //Para regresar al menu principal

    UPROPERTY(EditAnywhere, Category=HUD)
    TSubclassOf<class UUserWidget> ReturnToMainMenuWidget;

    UPROPERTY(EditAnywhere, Category = HUD)
    class UReturnToMainMenu* ReturnToMainMenu;

    bool bReturnToMainMenu = false;

	class AShooterHUD* ShooterHUD;

	float LevelStartingTime = 0.f;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CooldownTime = 0.f;

	uint32 CountdownInt = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;
	bool bInitializeCharacterOverlay = false;
	
    float HUDHealth;
	float HUDMaxHealth;
    bool bInitializeHealth = false;
	
    float HUDScore; 
    bool bInitializeScore = false;

	int32 HUDDefeats;
    bool bInitializeDefeats = false;

    int32 HUDGrenades;
    bool bInitializeGrenades = false;

    float HUDShield;
    float HUDMaxShield;
    bool bInitializeShield = false;

    float HUDCarriedAmmo;
    bool bInitializeCarriedAmmo = false;

    float HUDWeaponAmmo;
    bool bInitializeWeaponAmmo = false;

    float HighPingRunningTime = 0.f;

    UPROPERTY(EditAnywhere)
    float HighPingDuration = 5.f;

    float PingAnimRuningTime = 0.f;

    UPROPERTY(EditAnywhere)
    float CheckPingFrequency = 20.f;

    UFUNCTION(Server, Reliable)
    void ServerReportPingStatus(bool bHighPing);

    UPROPERTY(EditAnywhere)
    float HighPingThreshold = 50.f;
};
