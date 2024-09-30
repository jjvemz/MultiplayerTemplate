// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerGameMode.h"
#include "Multiplayertest/Character/ShooterPlayer.h"
#include "Multiplayertest/PlayerController/ShooterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"


void AShooterPlayerGameMode::PlayerElim(AShooterPlayer* EliminatedPlyr, AShooterPlayerController* VictimController, AShooterPlayerController* AttackerController)
{
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
