// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerController.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

#include "Multiplayertest/HUD/ShooterHUD.h"
#include "Multiplayertest/HUD/CharacterOverlay.h"
#include "Multiplayertest/HUD/Announcement.h"
#include "Multiplayertest/Character/ShooterPlayer.h"
#include "Multiplayertest/GameMode/ShooterPlayerGameMode.h"
#include "Multiplayertest/PlayerState/ShooterPlayerState.h"
#include "Multiplayertest/Components/CombatComponent.h"
#include "Multiplayertest/Weapons/WeaponActor.h"
#include "Multiplayertest/GameState/ShooterPlayerGameState.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


void AShooterPlayerController::BeginPlay()
{

	Super::BeginPlay();

	ShooterHUD = Cast<AShooterHUD>(GetHUD());
	ServerCheckMatchState();
}


void AShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AShooterPlayer* ShooterPlayer = Cast<AShooterPlayer>(InPawn);
	if (ShooterPlayer)
	{
		SetHUDHealth(ShooterPlayer->GetCurrHealth(), ShooterPlayer->GetMaxHealth());
	}
}

void AShooterPlayerController::SetHUDScore(float Score)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;

	if (ShooterHUD && ShooterHUD->CharacterOverlay
		&& ShooterHUD->CharacterOverlay->PointsText)
	{
		FString PointsText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		ShooterHUD->CharacterOverlay->PointsText->SetText(FText::FromString(PointsText));
	}
	else
	{
        bInitializeScore = true;
		HUDScore = Score;
	}
}


void AShooterPlayerController::SetHUDHealth(float CurrHealth, float MaxHealth)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;

	if (ShooterHUD && ShooterHUD->CharacterOverlay)
	{
		const float HealthPercent = CurrHealth / MaxHealth;
		ShooterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
	}
	else
	{
        bInitializeHealth = true;
		HUDHealth = CurrHealth;
		HUDMaxHealth = MaxHealth;
	}
}

void AShooterPlayerController::SetHUDShield(float CurrShield, float MaxShield)
{
    ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
    
    if (ShooterHUD &&
        ShooterHUD->CharacterOverlay &&
        ShooterHUD->CharacterOverlay->ShieldBar)
    {
        const float ShieldPercent = CurrShield / MaxShield;
        ShooterHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
    }
    else
    {
        bInitializeShield = true;
        HUDShield =CurrShield;
        HUDMaxShield = MaxShield;
    }
}



void AShooterPlayerController::SetHUDDefeats(int32 Defeats)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;

	if (ShooterHUD && ShooterHUD->CharacterOverlay
		&& ShooterHUD->CharacterOverlay->DefeatsAmount)
	{
		FString Defeat = FString::Printf(TEXT("%d"), Defeats);
		ShooterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(Defeat));
	}
	else
	{
        bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void AShooterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;

	if (ShooterHUD 
		&& ShooterHUD->CharacterOverlay
		&& ShooterHUD->CharacterOverlay->WeaponAmmoAmount)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		ShooterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
    else
    {
        bInitializeWeaponAmmo = true;
        HUDWeaponAmmo = Ammo;
    }
}

void AShooterPlayerController::SetHUDcarriedAmmo(int32 Ammo)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;

	if (ShooterHUD
		&& ShooterHUD->CharacterOverlay
		&& ShooterHUD->CharacterOverlay->MagCapacity)
	{
		FString MagText = FString::Printf(TEXT("%d"), Ammo);
		ShooterHUD->CharacterOverlay->MagCapacity->SetText(FText::FromString(MagText));
	}
    else
    {
        bInitializeCarriedAmmo = true;
        HUDCarriedAmmo = Ammo;
    }
}

void AShooterPlayerController::SetHUDGrenades(int32 Grenades)
{
    ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
    if (ShooterHUD &&
        ShooterHUD->CharacterOverlay &&
        ShooterHUD->CharacterOverlay->GrenadesText)
    {
        FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
        ShooterHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
    }
    else
    {
        bInitializeGrenades = true;
        HUDGrenades = Grenades;
    }
}

void AShooterPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;

	if (ShooterHUD
		&& ShooterHUD->CharacterOverlay
		&& ShooterHUD->CharacterOverlay->MatchCountdownText)
	{
		if (CountdownTime < 0.f)
		{
			ShooterHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		ShooterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void AShooterPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;

	if (ShooterHUD &&
		ShooterHUD->Announcement &&
		ShooterHUD->Announcement->WarmupTimeText)
	{
		if (CountdownTime < 0.f) {
			ShooterHUD->Announcement->WarmupTimeText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		ShooterHUD->Announcement->WarmupTimeText->SetText(FText::FromString(CountdownText));
	}
}

//Ping muy alto?
void AShooterPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
    HighPingDelegate.Broadcast(bHighPing);
}


void AShooterPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void AShooterPlayerController::HighPingWarning()
{
    ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;

    if (ShooterHUD &&
        ShooterHUD->CharacterOverlay &&
        ShooterHUD->CharacterOverlay->HighPingImage &&
        ShooterHUD->CharacterOverlay->HighPingAnim) 
    {
        ShooterHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
        ShooterHUD->CharacterOverlay->PlayAnimation(
            ShooterHUD->CharacterOverlay->HighPingAnim,
            0.f,
            5
        );
    }
}

void AShooterPlayerController::StopHighPingWarning()
{
    ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;

    if (ShooterHUD &&
        ShooterHUD->CharacterOverlay &&
        ShooterHUD->CharacterOverlay->HighPingImage &&
        ShooterHUD->CharacterOverlay->HighPingAnim)
    {
        ShooterHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
        if (ShooterHUD->CharacterOverlay->IsAnimationPlaying(ShooterHUD->CharacterOverlay->HighPingAnim))
        {
            ShooterHUD->CharacterOverlay->StopAnimation(ShooterHUD->CharacterOverlay->HighPingAnim);
        }
    }
}

void AShooterPlayerController::CheckPing(float DeltaTime)
{
    if (HasAuthority()) return;
    HighPingRunningTime += DeltaTime;
    if (HighPingRunningTime > CheckPingFrequency)
    {
        PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
        if (PlayerState) 
        {
            UE_LOG(LogTemp, Warning, TEXT("PlayerState->GetPing() * 4 : %d"), PlayerState->GetPing() * 4);
            if (PlayerState->GetPing() * 4 > HighPingThreshold)
            {
                HighPingWarning();
                PingAnimRuningTime = 0.f;
                ServerReportPingStatus(true);
            }
            else
            {
                ServerReportPingStatus(false);
            }
        }
        HighPingRunningTime = 0.f;
    }
    if (ShooterHUD && ShooterHUD->CharacterOverlay && ShooterHUD->CharacterOverlay->HighPingAnim
        && ShooterHUD->CharacterOverlay->IsAnimationPlaying(ShooterHUD->CharacterOverlay->HighPingAnim))
    {
        PingAnimRuningTime += DeltaTime;
        if (PingAnimRuningTime > HighPingDuration)
        {
            StopHighPingWarning();
        }
    }
}


void AShooterPlayerController::ServerCheckMatchState_Implementation()
{
	AShooterPlayerGameMode* ShooterPlayerGameMode = Cast<AShooterPlayerGameMode>(UGameplayStatics::GetGameMode(this));
	if (ShooterPlayerGameMode) 
	{
		MatchState = ShooterPlayerGameMode->GetMatchState();
		WarmupTime = ShooterPlayerGameMode->WarmupTime;
		MatchTime = ShooterPlayerGameMode->MatchTime;
		CooldownTime = ShooterPlayerGameMode->CooldownTime;
		LevelStartingTime = ShooterPlayerGameMode->LevelStartingTime;
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, LevelStartingTime, CooldownTime);
	}
}

void AShooterPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	CooldownTime = Cooldown;
	OnMatchStateSet(MatchState);
	if (ShooterHUD && MatchState == MatchState::WaitingToStart)
	{
		ShooterHUD->AddAnnouncement();
	}
}

void AShooterPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}

}


void AShooterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDTime();
	PollInit();
    CheckPing(DeltaTime);
}

void AShooterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShooterPlayerController, MatchState);
}

float AShooterPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AShooterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}
void AShooterPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AShooterPlayerController::HandleMatchHasStarted()
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;

	if (ShooterHUD)
	{
		if (ShooterHUD->CharacterOverlay == nullptr) ShooterHUD->AddCharacterOverlay();
		if (ShooterHUD->Announcement)
		{
			ShooterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AShooterPlayerController::HandleCooldown()
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;

	if (ShooterHUD)
	{
		ShooterHUD->CharacterOverlay->RemoveFromParent();
		if (ShooterHUD->Announcement &&
			ShooterHUD->Announcement->AnnouncementText &&
			ShooterHUD->Announcement->InfoAnnouncementText)
		{
			ShooterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText("La partida comenzara en: ");
			ShooterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			
			AShooterPlayerGameState* ShooterPlayerGameState = Cast<AShooterPlayerGameState>(UGameplayStatics::GetGameState(this));
			AShooterPlayerState* ShooterPlayerState = GetPlayerState<AShooterPlayerState>();
			if (ShooterPlayerGameState && ShooterPlayerState)
			{
				TArray<AShooterPlayerState*> TopPlayers = ShooterPlayerGameState->TopScoringPlayers;
				FString InfoTextString;
				if (TopPlayers.Num() == 0)
				{
					InfoTextString = FString("No hubo ganador.");
				}
				else if (TopPlayers.Num() == 1 && TopPlayers[0] == ShooterPlayerState)
				{
					InfoTextString = FString("Ganaste!");
				}
				else if (TopPlayers.Num() == 1)
				{
					InfoTextString = FString::Printf(TEXT("El ganador es: \n%s"), *TopPlayers[0]->GetPlayerName());
				}
				else if (TopPlayers.Num() > 1)
				{
					InfoTextString = FString("Los ganadores son:\n");
					for (auto TiedPlayer : TopPlayers)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
					}
				}
				ShooterHUD->Announcement->InfoAnnouncementText->SetText(FText::FromString(InfoTextString));
			}
		}
	}
	AShooterPlayer* ShooterPlayer = Cast<AShooterPlayer>(GetPawn());
	if (ShooterPlayer && ShooterPlayer->GetCombat())
	{
		ShooterPlayer->bDisableGameplay = true;
		ShooterPlayer->GetCombat()->FireButtonPressed(false);
	}
}

void AShooterPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() - LevelStartingTime;
	else if(MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() - LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}
	CountdownInt = SecondsLeft;
}

void AShooterPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (ShooterHUD && ShooterHUD->CharacterOverlay)
		{
			CharacterOverlay = ShooterHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
                if(bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);

                if (bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield);

                if (bInitializeScore) SetHUDScore(HUDScore);

                if (bInitializeDefeats) SetHUDDefeats(HUDDefeats);

                if (bInitializeCarriedAmmo) SetHUDcarriedAmmo(HUDCarriedAmmo);

                if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);

                AShooterPlayer* ShooterPlayer = Cast<AShooterPlayer>(GetPawn());
                if (ShooterPlayer && ShooterPlayer->GetCombat())
                {
                    if (bInitializeGrenades) SetHUDGrenades(ShooterPlayer->GetCombat()->GetGrenades());
                }
			}
		}
	}
}

void AShooterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void AShooterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
    SingleTripTime = (0.5f * RoundTripTime);
	float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

