// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ShooterPlayerGameMode.generated.h"

namespace MatchState
{
	extern MULTIPLAYERTEST_API const FName Cooldown; // Para cuando la duración de la partida haya concluido y se muestre al jugador
}
/**
 * 
 */
UCLASS()
class MULTIPLAYERTEST_API AShooterPlayerGameMode : public AGameMode
{
	GENERATED_BODY()
	public:
	AShooterPlayerGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerElim(class AShooterPlayer* EliminatedPlyr, 
	class AShooterPlayerController* VictimController, class AShooterPlayerController* AttackerController );
	virtual void RequestRespawn(class ACharacter* EliminatedCharacter, AController* EliminatedController);
    void PlayerLeftGame(class AShooterPlayerState* PlayerLeaving);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 180.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:

	float CountdownTime = 0.f;
};
