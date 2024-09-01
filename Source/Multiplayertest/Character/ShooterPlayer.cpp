// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Multiplayertest/Weapon.h"
#include "Multiplayertest/Components/CombatComponent.h"


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
	OverHeadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverHeadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
}

void AShooterPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AShooterPlayer, OverlappingWeapon, COND_OwnerOnly);
}

void AShooterPlayer::BeginPlay()
{
	Super::BeginPlay();
	
}

void AShooterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &AShooterPlayer::EquippedPressedButton);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooterPlayer::CrouchButtonPressed);

	PlayerInputComponent->BindAxis("Forward", this, &AShooterPlayer::Forward);
	PlayerInputComponent->BindAxis("Right", this, &AShooterPlayer::Right);
	PlayerInputComponent->BindAxis("Turn", this, &AShooterPlayer::Turn);
	PlayerInputComponent->BindAxis("Yaw", this, &AShooterPlayer::Yaw);

}

void AShooterPlayer::PostInitializeComponents() 
{
	Super::PostInitializeComponents();
		if (Combat)
		{
			Combat->Character = this;
		}
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

void AShooterPlayer::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void AShooterPlayer::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void AShooterPlayer::EquippedPressedButton() 
{
	if (Combat )
	{
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);

		}
		else
		{
			ServerEquippedButtonPressed();
		}
	}

}

void AShooterPlayer::ServerEquippedButtonPressed_Implementation() 
{
	if (Combat && HasAuthority())
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}

}

bool AShooterPlayer::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);

}

void AShooterPlayer::CrouchButtonPressed()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AShooterPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


}


