// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayer.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

#include "Multiplayertest/Weapons/WeaponActor.h"
#include "Multiplayertest/Components/CombatComponent.h"
#include "Multiplayertest/Multiplayertest.h"
#include "Multiplayertest/PlayerController/ShooterPlayerController.h"
#include "Multiplayertest/GameMode/ShooterPlayerGameMode.h"

#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"


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
	/*
	OverHeadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverHeadWidget->SetupAttachment(RootComponent);
	OverHeadWidget->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	*/

	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComp->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);


	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}


void AShooterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &AShooterPlayer::EquippedPressedButton);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooterPlayer::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AShooterPlayer::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AShooterPlayer::AimButtonReleased);
	PlayerInputComponent->BindAction("Shoot", IE_Released, this, &AShooterPlayer::FireButtonReleased);
	PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &AShooterPlayer::FireButtonPressed);


	PlayerInputComponent->BindAxis("Forward", this, &AShooterPlayer::Forward);
	PlayerInputComponent->BindAxis("Right", this, &AShooterPlayer::Right);
	PlayerInputComponent->BindAxis("Turn", this, &AShooterPlayer::Turn);
	PlayerInputComponent->BindAxis("Yaw", this, &AShooterPlayer::Yaw);

}
void AShooterPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AShooterPlayer, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(AShooterPlayer, CurrHealth);
}

void AShooterPlayer::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void AShooterPlayer::Elim()
{
	if (CombatComp && CombatComp->EquippedWeapon)
	{
		CombatComp->EquippedWeapon->DroppedWeapon();
	}
	MulticastElim();
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&AShooterPlayer::ElimTimerFinished,
		ElimDelay
	);
}

void AShooterPlayer::MulticastElim_Implementation()
{
	bIsEliminated = true;
	PlayEliminationMontage();

	//Quitar el movimiento al ser eliminado

	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (ShooterPlayerController)
	{
		DisableInput(ShooterPlayerController);
	}
	
	//Quitar las colisiones
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AShooterPlayer::BeginPlay()
{
	Super::BeginPlay();

	UpdateHUDHealth();
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AShooterPlayer::ReceiveDamage);
	}
}

void AShooterPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.10f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
	HideCameraIfTheCaharacterisClose();
}

void AShooterPlayer::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CombatComp)
	{
		CombatComp->Character = this;
	}
}

void AShooterPlayer::PlayShootingMontage(bool bAiming)
{

	if (CombatComp == nullptr || CombatComp->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimationInstance = GetMesh()->GetAnimInstance();

	if (AnimationInstance && FireWeaponMontage)
	{
		AnimationInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimationInstance->Montage_JumpToSection(SectionName);
	}
}

void AShooterPlayer::Forward(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void AShooterPlayer::Right(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void AShooterPlayer::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AShooterPlayer::Yaw(float Value)
{
	AddControllerPitchInput(Value);
}

void AShooterPlayer::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void AShooterPlayer::EquippedPressedButton()
{
	if (CombatComp)
	{
		if (HasAuthority())
		{
			CombatComp->EquipWeapon(OverlappingWeapon);

		}
		else
		{
			ServerEquippedButtonPressed();
		}
	}

}

void AShooterPlayer::ServerEquippedButtonPressed_Implementation()
{
	if (CombatComp)
	{
		CombatComp->EquipWeapon(OverlappingWeapon);
	}

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

void AShooterPlayer::AimButtonPressed()
{
	if (CombatComp)
	{
		CombatComp->SetAiming(true);
	}
}

void AShooterPlayer::AimButtonReleased()
{
	if (CombatComp)
	{
		CombatComp->SetAiming(false);
	}
}

float AShooterPlayer::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void AShooterPlayer::AimOffset(float DeltaTime)
{
	if (CombatComp && CombatComp->EquippedWeapon == nullptr) return;
	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir)
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir)
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterptAO_Way = FMath::FInterpTo(InterptAO_Way, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterptAO_Way;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}

	CalculateAO_Pitch();
}

void AShooterPlayer::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}


void AShooterPlayer::PlayHitReactMontage()
{

	if (CombatComp == nullptr || CombatComp->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimationInstance = GetMesh()->GetAnimInstance();

	if (AnimationInstance && HitReactMontage)
	{
		AnimationInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimationInstance->Montage_JumpToSection(SectionName);
	}
}

void AShooterPlayer::PlayEliminationMontage()
{
	UAnimInstance* AnimationInstance = GetMesh()->GetAnimInstance();

	if (AnimationInstance && EliminationMontage)
	{
		AnimationInstance->Montage_Play(EliminationMontage);
	}
}

void AShooterPlayer::ReceiveDamage(AActor* DamagedACtor, float Damage, const UDamageType* DamageType, 
	AController* InstigatorController, AActor* DamageCausor)
{
	CurrHealth = FMath::Clamp(CurrHealth - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();
	PlayHitReactMontage();
	AShooterPlayerGameMode* ShooterGameMode= GetWorld()->GetAuthGameMode<AShooterPlayerGameMode>();

	if (CurrHealth == 0.f)
	{
		if (ShooterGameMode)
		{
			ShooterPlayerController = ShooterPlayerController == NULL ? 
				Cast<AShooterPlayerController>(Controller) 
				: ShooterPlayerController;
			AShooterPlayerController* AttackerController = Cast<AShooterPlayerController>(InstigatorController);
			//ShooterGameMode->PlayerElim(this, ShooterPlayerController, AttackerController);
			ShooterGameMode->PlayerElim(this, ShooterPlayerController, AttackerController);

		}
	}
	
}

void AShooterPlayer::UpdateHUDHealth()
{
	ShooterPlayerController = ShooterPlayerController == nullptr ? Cast<AShooterPlayerController>(Controller) : ShooterPlayerController;
	if (ShooterPlayerController)
	{
		ShooterPlayerController->SetHUDHealth(CurrHealth, MaxHealth);
	}
}

void AShooterPlayer::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
}


void AShooterPlayer::SimProxiesTurn()
{
	if (CombatComp == nullptr && CombatComp->EquippedWeapon == nullptr) return;
	
	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if (Speed > 0.f) {
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;


	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;

		}
		else 
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;

		}
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void AShooterPlayer::FireButtonPressed()
{
	if (CombatComp)
	{
		CombatComp->FireButtonPressed(true);
	}
}

void AShooterPlayer::FireButtonReleased()
{
	if (CombatComp)
	{
		CombatComp->FireButtonPressed(false);
	}
}
/*
void AShooterPlayer::MulticastHit_Implementation()
{
	PlayHitReactMontage();
}
*/
void AShooterPlayer::HideCameraIfTheCaharacterisClose()
{
	if (!IsLocallyControlled()) return;

	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size()< CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (CombatComp && CombatComp->EquippedWeapon && CombatComp->EquippedWeapon->GetWeaponMesh())
		{
			CombatComp->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
		
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (CombatComp && CombatComp->EquippedWeapon && CombatComp->EquippedWeapon->GetWeaponMesh())
		{
			CombatComp->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void AShooterPlayer::ElimTimerFinished()
{
	AShooterPlayerGameMode* ShooterPlayerGameMode = GetWorld()->GetAuthGameMode<AShooterPlayerGameMode>();
	if (ShooterPlayerGameMode)
	{
		ShooterPlayerGameMode->RequestRespawn(this, Controller);
	}
}

void AShooterPlayer::OnRep_Health()
{
	UpdateHUDHealth();
	PlayHitReactMontage();
}

void AShooterPlayer::SetOverlappingWeapon(AWeaponActor* Weapon)
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

void AShooterPlayer::OnRep_OverlappingWeapon(AWeaponActor* LastWeapon)
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

bool AShooterPlayer::IsWeaponEquipped()
{
	return (CombatComp && CombatComp->EquippedWeapon);
}

bool AShooterPlayer::IsAiming()
{
	return (CombatComp && CombatComp->bAiming);
}

AWeaponActor* AShooterPlayer::GetEquippedWeapon()
{
	if (CombatComp == nullptr) return nullptr;
	return CombatComp->EquippedWeapon;
}

FVector AShooterPlayer::GetHitTarget() const
{
	if (CombatComp == nullptr) return FVector();

	return CombatComp->HitTarget;
}
