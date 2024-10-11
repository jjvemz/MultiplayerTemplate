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
#include "Multiplayertest/Weapons/WeaponTypes.h"
#include "Multiplayertest/BlasterTypes/CombatState.h"

#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"


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

    AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
    AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
    AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
    PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AShooterPlayer::ReloadButtonPressed);
    PlayerInputComponent->BindAction("Grenade", IE_Pressed, this, &AShooterPlayer::GrenadeButtonPressed);



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
    DOREPLIFETIME(AShooterPlayer, bDisableGameplay);

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

void AShooterPlayer::Destroyed()
{
    Super::Destroyed();

    if (ElimBotComponent)
    {
        ElimBotComponent->DestroyComponent();
    }

    AShooterPlayerGameMode* ShooterPlayerGameMode = Cast<AShooterPlayerGameMode>(UGameplayStatics::GetGameMode(this));
    bool bMatchNotInProgress = ShooterPlayerGameMode && ShooterPlayerGameMode->GetMatchState() != MatchState::InProgress;
    if (CombatComp && CombatComp->EquippedWeapon && bMatchNotInProgress)
    {
        CombatComp->EquippedWeapon->Destroy();
    }
}

void AShooterPlayer::MulticastElim_Implementation()
{
    if (ShooterPlayerController)
    {
        ShooterPlayerController->SetHUDWeaponAmmo(0);
    }

    bIsEliminated = true;
    PlayEliminationMontage();

    //Quitar el movimiento al ser eliminado
    bDisableGameplay = true;
    GetCharacterMovement()->DisableMovement();
    if (CombatComp)
    {
        CombatComp->FireButtonPressed(false);
    }

    //Quitar las colisiones
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    //Spawnear Elimbot

    if (ElimBotEffect)
    {
        FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
        ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            ElimBotEffect,
            ElimBotSpawnPoint,
            GetActorRotation()
        );
    }
    if (ElimBotSound)
    {
        UGameplayStatics::SpawnSoundAtLocation(
            this,
            ElimBotSound,
            GetActorLocation()
        );
    }

    bool bHideScope = IsLocallyControlled() &&
        CombatComp &&
        CombatComp->bAiming &&
        CombatComp->EquippedWeapon &&
        CombatComp->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;

    if (bHideScope) {
        ShowSniperScopeWidget(false);
    }

}


void AShooterPlayer::BeginPlay()
{
    Super::BeginPlay();

    UpdateHUDHealth();
    if (HasAuthority())
    {

        OnTakeAnyDamage.AddDynamic(this, &AShooterPlayer::ReceiveDamage);

    }
    if (AttachedGrenade)
    {

        AttachedGrenade->SetVisibility(false);

    }
}

void AShooterPlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    RotateInPlace(DeltaTime);
    HideCameraIfTheCaharacterisClose();
    PollInit();
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
    if (bDisableGameplay) return;
    if (Controller != nullptr && Value != 0.f)
    {
        const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
        const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
        AddMovementInput(Direction, Value);
    }
}

void AShooterPlayer::Right(float Value)
{
    if (bDisableGameplay) return;
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
    if (bDisableGameplay) return;
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
    if (bDisableGameplay) return;
    if (CombatComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("Deberia tomar el arma"));
        if (HasAuthority())
        {
            CombatComp->EquipWeapon(OverlappingWeapon);
            UE_LOG(LogTemp, Warning, TEXT("EquippedWeapon is set to: %s"), *CombatComp->GetClass()->GetName());

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
    if (bDisableGameplay) return;
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
    if (bDisableGameplay) return;
    if (CombatComp)
    {
        CombatComp->SetAiming(true);
    }
}

void AShooterPlayer::AimButtonReleased()
{
    if (bDisableGameplay) return;
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

void AShooterPlayer::PlayReloadMontage()
{
    if (CombatComp == nullptr || CombatComp->EquippedWeapon == nullptr) return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

    if (AnimInstance && ReloadMontage)
    {
        AnimInstance->Montage_Play(ReloadMontage);
        FName SectionName;

        switch (CombatComp->EquippedWeapon->GetWeaponType())
        {

        case EWeaponType::EWT_AssaultRifle:
            SectionName = FName("Rifle");
            break;

        case EWeaponType::EWT_RocketLauncher:
            SectionName = FName("RocketLauncher");
            break;

        case EWeaponType::EWT_Pistol:
            SectionName = FName("Pistol");
            break;

        case EWeaponType::EWT_SubmachineGun:
            SectionName = FName("Pistol");
            break;

        case EWeaponType::EWT_Shotgun:
            SectionName = FName("Shotgun");
            break;

        case EWeaponType::EWT_SniperRifle:
            SectionName = FName("SniperRifle");
            break;

        case EWeaponType::EWT_GrenadeLauncher:
            SectionName = FName("GrenadeLauncher");
            break;
        }


        AnimInstance->Montage_JumpToSection(SectionName);
    }
}

void AShooterPlayer::PlayThrowGrenadeMontage()
{
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance && ThrowGrenadeMontage)
    {
        AnimInstance->Montage_Play(ThrowGrenadeMontage);
    }
}

void AShooterPlayer::ReceiveDamage(AActor* DamagedACtor, float Damage, const UDamageType* DamageType,
    AController* InstigatorController, AActor* DamageCausor)
{
    CurrHealth = FMath::Clamp(CurrHealth - Damage, 0.f, MaxHealth);
    UpdateHUDHealth();
    PlayHitReactMontage();
    AShooterPlayerGameMode* ShooterGameMode = GetWorld()->GetAuthGameMode<AShooterPlayerGameMode>();

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

void AShooterPlayer::PollInit()
{
    if (ShooterPlayerState == nullptr)
    {
        ShooterPlayerState = GetPlayerState<AShooterPlayerState>();
        if (ShooterPlayerState)
        {
            ShooterPlayerState->AddToScore(0.f);
            ShooterPlayerState->AddToDefeats(0);

        }
    }
}

void AShooterPlayer::RotateInPlace(float DeltaTime)
{

    if (bDisableGameplay)
    {
        bUseControllerRotationYaw = false;
        TurningInPlace = ETurningInPlace::ETIP_NotTurning;
        return;
    }
    if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
    {
        AimOffset(DeltaTime);
    }
    else
    {
        TimeSinceLastMovementReplication += DeltaTime;
        if (TimeSinceLastMovementReplication > 0.25f)
        {
            OnRep_ReplicatedMovement();
        }
        CalculateAO_Pitch();
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
    if (CombatComp == nullptr || CombatComp->EquippedWeapon == nullptr) return;

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
        else if (ProxyYaw < -TurnThreshold)
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
    if (bDisableGameplay) return;
    if (CombatComp)
    {
        CombatComp->FireButtonPressed(true);
    }
}

void AShooterPlayer::FireButtonReleased()
{
    if (bDisableGameplay) return;
    if (CombatComp)
    {
        CombatComp->FireButtonPressed(false);
    }
}

void AShooterPlayer::ReloadButtonPressed()
{
    if (bDisableGameplay) return;
    if (CombatComp)
    {
        CombatComp->Reload();
    }
}

void AShooterPlayer::GrenadeButtonPressed()
{
    if (CombatComp)
    {
        CombatComp->ThrowGrenade();
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

    if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
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

ECombatState AShooterPlayer::GetCombatState() const
{
    if (CombatComp == nullptr) return ECombatState::ECS_MAX;
    return CombatComp->CombatState;
}

FVector AShooterPlayer::GetHitTarget() const
{
    if (CombatComp == nullptr) return FVector();

    return CombatComp->HitTarget;
}
