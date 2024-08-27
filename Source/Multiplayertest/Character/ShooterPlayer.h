// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ShooterPlayer.generated.h"


UCLASS()
class MULTIPLAYERTEST_API AShooterPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	AShooterPlayer();
	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	
	virtual void BeginPlay() override;
	void Forward(float value);
	void Right(float value);
	void Turn(float value);
	void Yaw(float value);

private:
	UPROPERTY(VisibleAnywhere, Category=Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

public:

};
