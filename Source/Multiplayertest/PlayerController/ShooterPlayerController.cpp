// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerController.h"
#include "Components/ProgressBar.h"
#include "Multiplayertest/HUD/ShooterHUD.h"
#include "Multiplayertest/HUD/CharacterOverlay.h"



void AShooterPlayerController::BeginPlay()
{

	Super::BeginPlay();

	ShooterHUD = Cast<AShooterHUD>(GetHUD());
}

void AShooterPlayerController::SetHUDHealth(float CurrHealth, float MaxHealth)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;

	if (ShooterHUD && ShooterHUD->CharacterOverlay)
	{
		const float HealthPercent = CurrHealth / MaxHealth;
		ShooterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
	}
}