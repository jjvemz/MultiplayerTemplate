// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ShooterPlayerGameState.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTEST_API AShooterPlayerGameState : public AGameState
{
	GENERATED_BODY()
	
public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(class AShooterPlayerState* ScoringPlayer);

	UPROPERTY(Replicated)
	TArray<AShooterPlayerState*> TopScoringPlayers;

private:
	float TopScore = 0.f;
};
