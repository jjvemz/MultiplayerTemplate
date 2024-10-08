// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerGameMode.h"
#include "Multiplayertest/Character/ShooterPlayer.h"
#include "Multiplayertest/PlayerController/ShooterPlayerController.h"
#include "Multiplayertest/PlayerState/ShooterPlayerState.h"
#include "Multiplayertest/GameState/ShooterPlayerGameState.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}



AShooterPlayerGameMode::AShooterPlayerGameMode()
{
	bDelayedStart = true;
}

void AShooterPlayerGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetRealTimeSeconds();
}

void AShooterPlayerGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AShooterPlayerController* ShooterPlayer = Cast<AShooterPlayerController>(*It);
		if (ShooterPlayer)
		{
			ShooterPlayer->OnMatchStateSet(MatchState);
		}
	}
}

void AShooterPlayerGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress) 
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}

}

void AShooterPlayerGameMode::PlayerElim(AShooterPlayer* EliminatedPlyr, AShooterPlayerController* VictimController, AShooterPlayerController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;

	AShooterPlayerState* AttackerPlayerState = AttackerController ? Cast<AShooterPlayerState>(AttackerController->PlayerState) : nullptr;
	AShooterPlayerState* VictimPlayerState = VictimController ? Cast<AShooterPlayerState>(VictimController->PlayerState) : nullptr;

	AShooterPlayerGameState* ShooterPlayerGameState = GetGameState<AShooterPlayerGameState>();
	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(5.f);
		ShooterPlayerGameState->UpdateTopScore(AttackerPlayerState);
	}
	
	if(VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
		VictimPlayerState->AddToScore(-2.5f);

	}

	if (EliminatedPlyr)
	{
		EliminatedPlyr->Elim();
	}
}

void AShooterPlayerGameMode::RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController)
{

	if (EliminatedCharacter)
	{
		EliminatedCharacter->Reset();
		EliminatedCharacter->Destroy();
	}

	if (EliminatedController)
	{
		TArray<AActor*> LevelPlayer;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), LevelPlayer);
		int32 Selection = FMath::RandRange(0, LevelPlayer.Num()-1);
		RestartPlayerAtPlayerStart(EliminatedController, LevelPlayer[Selection]);
	}
}

