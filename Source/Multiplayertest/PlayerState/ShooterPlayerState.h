// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ShooterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTEST_API AShooterPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	virtual void OnRep_Defeats();

	virtual void OnRep_Score() override;

	void AddToScore(float ScoreAmount);
	void AddToDefeats(int32 DefeatsAmount);

private:
	class AShooterPlayer* Character;
	class AShooterPlayerController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;
};
