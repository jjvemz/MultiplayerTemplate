// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"



AShooterPlayer::AShooterPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void AShooterPlayer::BeginPlay()
{
	Super::BeginPlay();
	
}

void AShooterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAxis("Forward", this, &AShooterPlayer::Forward);
	PlayerInputComponent->BindAxis("Right", this, &AShooterPlayer::Right);
	PlayerInputComponent->BindAxis("Turn", this, &AShooterPlayer::Turn);
	PlayerInputComponent->BindAxis("Yaw", this, &AShooterPlayer::Yaw);



}

void AShooterPlayer::Forward(float value)
{
	if (Controller != nullptr && value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, value);
	}
}

void AShooterPlayer::Right(float value)
{
	if (Controller != nullptr && value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, value);
	}
}

void AShooterPlayer::Turn(float value)
{
	AddControllerYawInput(value);
}

void AShooterPlayer::Yaw(float value)
{
	AddControllerPitchInput(value);

}

void AShooterPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


