// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerState.h"
#include "Multiplayertest/Character/ShooterPlayer.h"
#include "Multiplayertest/PlayerController/ShooterPlayerController.h"
#include "Net/UnrealNetwork.h"


void AShooterPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShooterPlayerState, Defeats);
}

void AShooterPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(Score += ScoreAmount);
	Character = Character == nullptr ? Cast<AShooterPlayer>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;

		if (Controller)
		{
			Controller->SetHUDScore(Score);
		}
	}
}

void AShooterPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	Character = Character == nullptr ? Cast<AShooterPlayer>(GetPawn()) : Character;
	if (Character && Character->Controller)
	{
		Controller = Controller == NULL ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}


void AShooterPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<AShooterPlayer>(GetPawn()) : Character;
	if (Character && Character->Controller)
	{
		Controller = Controller == NULL ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);

		}
	}
}

void AShooterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<AShooterPlayer>(GetPawn()) : Character;

	if (Character)
	{
		Controller = Controller == NULL ? Cast<AShooterPlayerController>(Character->Controller): Controller;

		if (Controller)
		{
			Controller->SetHUDScore(Score);

		}
	}
}

