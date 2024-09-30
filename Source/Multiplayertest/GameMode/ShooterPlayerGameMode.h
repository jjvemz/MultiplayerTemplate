// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ShooterPlayerGameMode.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTEST_API AShooterPlayerGameMode : public AGameMode
{
	GENERATED_BODY()
	public:
	virtual void PlayerElim(class AShooterPlayer* EliminatedPlyr, 
		class AShooterPlayerController* VictimController, class AShooterPlayerController* AttackerController );
	virtual void RequestRespawn(class ACharacter* EliminatedCharacter, AController* EliminatedController);
};
