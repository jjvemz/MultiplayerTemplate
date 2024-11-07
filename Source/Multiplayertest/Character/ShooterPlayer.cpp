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
#include "Multiplayertest/Components/LagCompensationComponent.h"
#include "Multiplayertest/Components/BuffComponent.h"

#include "Multiplayertest/Multiplayertest.h"
#include "Multiplayertest/PlayerController/ShooterPlayerController.h"
#include "Multiplayertest/GameMode/ShooterPlayerGameMode.h"
#include "Multiplayertest/GameState/ShooterPlayerGameState.h"
#include "Multiplayertest/Weapons/WeaponTypes.h"
#include "Multiplayertest/BlasterTypes/CombatState.h"

#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/BoxComponent.h"


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

    BuffComp = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
    BuffComp->SetIsReplicated(true);
     
    LagCompensationComp = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensationComponent"));


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

    head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
    head->SetupAttachment(GetMesh(), FName("head"));
    HitCollisionBoxes.Add(FName("head"), head);

    pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("Pelvis"));
    pelvis->SetupAttachment(GetMesh(), FName("Pelvis"));
    HitCollisionBoxes.Add(FName("Pelvis"), pelvis);

    spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
    spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
    HitCollisionBoxes.Add(FName("spine_02"), spine_02);


    spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
    spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
    HitCollisionBoxes.Add(FName("spine_03"), spine_03);

    
    upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("UpperArm_L"));
    upperarm_l->SetupAttachment(GetMesh(), FName("UpperArm_L"));
    HitCollisionBoxes.Add(FName("UpperArm_L"), upperarm_l);

    
    upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("UpperArm_R"));
    upperarm_r->SetupAttachment(GetMesh(), FName("UpperArm_R"));
    HitCollisionBoxes.Add(FName("UpperArm_R"), upperarm_r);

    lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
    lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
    HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

    
    lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
    lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
    HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

    hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("Hand_L"));
    hand_l->SetupAttachment(GetMesh(), FName("Hand_L"));
    HitCollisionBoxes.Add(FName("Hand_L"), hand_l);

    
    hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("Hand_R"));
    hand_r->SetupAttachment(GetMesh(), FName("Hand_R"));
    HitCollisionBoxes.Add(FName("Hand_R"), hand_r);

    
    blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket"));
    blanket->SetupAttachment(GetMesh(), FName("backpack"));
    HitCollisionBoxes.Add(FName("blanket"), blanket);

    
    backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
    backpack->SetupAttachment(GetMesh(), FName("backpack"));
    HitCollisionBoxes.Add(FName("backpack"), backpack);

    
    thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("Thigh_L"));
    thigh_l->SetupAttachment(GetMesh(), FName("Thigh_L"));
    HitCollisionBoxes.Add(FName("Thigh_L"), thigh_l);

    
    thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("Thigh_R"));
    thigh_r->SetupAttachment(GetMesh(), FName("Thigh_R"));
    HitCollisionBoxes.Add(FName("Thigh_R"), thigh_r);

    
    calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
    calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
    HitCollisionBoxes.Add(FName("calf_l"), calf_l);

    
    calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
    calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
    HitCollisionBoxes.Add(FName("calf_r"), calf_r);

    
    foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("Foot_L"));
    foot_l->SetupAttachment(GetMesh(), FName("Foot_L"));
    HitCollisionBoxes.Add(FName("Foot_L"), foot_l);
    
    foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("Foot_R"));
    foot_r->SetupAttachment(GetMesh(), FName("Foot_R"));
    HitCollisionBoxes.Add(FName("Foot_R"), foot_r);


    for (auto Box : HitCollisionBoxes)
    {
        if (Box.Value)
        {
            Box.Value->SetCollisionObjectType(ECC_HitBox);
            Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
            Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
            Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
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
    DOREPLIFETIME(AShooterPlayer, CurrShield);
    DOREPLIFETIME(AShooterPlayer, bDisableGameplay);

}

void AShooterPlayer::OnRep_ReplicatedMovement()
{
    Super::OnRep_ReplicatedMovement();
    SimProxiesTurn();
    TimeSinceLastMovementReplication = 0.f;
}

void AShooterPlayer::Elim(bool bPlayerLeftGame)
{
    
    DropOrDestroyWeapons();
    MulticastElim(bPlayerLeftGame);
    
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

void AShooterPlayer::MulticastElim_Implementation(bool bPlayerLeftGame)
{
    bLeftGame = bPlayerLeftGame;
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

    if (CrownComponent)
    {
        CrownComponent->DestroyComponent();
    }

    GetWorldTimerManager().SetTimer(
        ElimTimer,
        this,
        &AShooterPlayer::ElimTimerFinished,
        ElimDelay
    );

}


void AShooterPlayer::MulticastGainedTheLead_Implementation()
{
    if (CrownSystem == nullptr) return;
    if (CrownComponent == nullptr)
    {
        CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
            CrownSystem,
            GetCapsuleComponent(),
            FName(),
            GetActorLocation() + FVector(0.f, 0.f, 110.f),
            GetActorRotation(),
            EAttachLocation::KeepWorldPosition,
            false
        );
    }
    if (CrownComponent)
    {
        CrownComponent->Activate();
    }
}

void AShooterPlayer::MulticastLostTheLead_Implementation()
{
    if (CrownComponent)
    {
        CrownComponent->DestroyComponent();
    }
}

void AShooterPlayer::BeginPlay()
{
    Super::BeginPlay();

    SpawnDefaultWeapon();
    UpdateHUDAmmo();
    UpdateHUDHealth();
    UpdateHUDShield();
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
    if (BuffComp)
    {
        BuffComp->Character = this;
        BuffComp->SetInitialSpeeds(
            GetCharacterMovement()->MaxWalkSpeed,
            GetCharacterMovement()->MaxWalkSpeedCrouched
        );
        BuffComp->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
    }
    if (LagCompensationComp)
    {
        LagCompensationComp->ShooterCharacter = this;
        if (Controller)
        {
            LagCompensationComp->ShooterController = Cast<AShooterPlayerController>(Controller);
        }
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
        if(CombatComp->CombatState == ECombatState::ECS_Unoccupied) ServerEquippedButtonPressed();
        if (CombatComp->ShouldSwapWeapons() 
            && !HasAuthority() 
            && CombatComp->CombatState == ECombatState::ECS_Unoccupied
            && OverlappingWeapon == nullptr) 
        {
            PlaySwapWeapon();
            CombatComp->CombatState = ECombatState::ECS_SwappingWeapons;
            bFinishedSwapping = false;
        }
    }
}

void AShooterPlayer::ServerEquippedButtonPressed_Implementation()
{
    if (CombatComp)
    {
        CombatComp->EquipWeapon(OverlappingWeapon);
        if (OverlappingWeapon) 
        {
            CombatComp->EquipWeapon(OverlappingWeapon);
        }
        else if (CombatComp->ShouldSwapWeapons())
        {
            CombatComp->SwapWeapons();
        }
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

void AShooterPlayer::DropOrDestroyWeapon(AWeaponActor* Weapon)
{
    if (Weapon == nullptr) return;

    if (Weapon->bDestroyWeapon)
    {
        Weapon->Destroy();
    }
    else
    {
        Weapon->DroppedWeapon();
    }
}

void AShooterPlayer::DropOrDestroyWeapons()
{
    if (CombatComp)
    {
        if (CombatComp->EquippedWeapon)
        {
            DropOrDestroyWeapon(CombatComp->EquippedWeapon);
        }
        if (CombatComp->SecondaryWeapon)
        {
            DropOrDestroyWeapon(CombatComp->SecondaryWeapon);
        }
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

void AShooterPlayer::PlaySwapWeapon()
{
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance && WeaponSwapMontage)
    {
        AnimInstance->Montage_Play(WeaponSwapMontage);
    }
}

void AShooterPlayer::ReceiveDamage(AActor* DamagedACtor, float Damage, const UDamageType* DamageType,
    AController* InstigatorController, AActor* DamageCausor)
{
    if (bIsEliminated) return;

    float DamageToHealth = Damage;
    if (CurrShield > 0.f) 
    {
        if (CurrShield >= Damage)
        {
            CurrShield = FMath::Clamp(CurrShield - Damage, 0.f, MaxShield);
            DamageToHealth = 0.f;
        }
        else 
        {
            CurrShield = FMath::Clamp(DamageToHealth, 0.f, Damage);
            CurrShield = 0;


        }
    }
    CurrHealth = FMath::Clamp(CurrHealth - DamageToHealth, 0.f, MaxHealth);

    UpdateHUDHealth();
    UpdateHUDShield();
    PlayHitReactMontage();


    if (CurrHealth == 0.f)
    {
        AShooterPlayerGameMode* ShooterGameMode = GetWorld()->GetAuthGameMode<AShooterPlayerGameMode>();

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

void AShooterPlayer::UpdateHUDShield()
{
    ShooterPlayerController = ShooterPlayerController == nullptr ? Cast<AShooterPlayerController>(Controller) : ShooterPlayerController;
    if (ShooterPlayerController)
    {
        ShooterPlayerController->SetHUDShield(CurrShield, MaxShield);
    }
}

void AShooterPlayer::UpdateHUDAmmo()
{
    ShooterPlayerController = ShooterPlayerController == nullptr ? Cast<AShooterPlayerController>(Controller) : ShooterPlayerController;
    if (ShooterPlayerController && CombatComp && CombatComp->EquippedWeapon)
    {
        ShooterPlayerController->SetHUDcarriedAmmo(CombatComp->CarriedAmmo);
        ShooterPlayerController->SetHUDWeaponAmmo(CombatComp->EquippedWeapon->GetAmmo());

    }

}

void AShooterPlayer::SpawnDefaultWeapon()
{
    AShooterPlayerGameMode* ShooterPlayerGameMode = Cast<AShooterPlayerGameMode>(UGameplayStatics::GetGameMode(this));
    UWorld* World = GetWorld();

    if (ShooterPlayerGameMode && World && !bIsEliminated && DefaultWeapon)
    {
        AWeaponActor* StartingWeapon = World->SpawnActor<AWeaponActor>(DefaultWeapon);
        if (CombatComp)
        {
            CombatComp->EquipWeapon(StartingWeapon);
        }
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

            AShooterPlayerGameState* ShooterPlayerGameState = Cast<AShooterPlayerGameState>(UGameplayStatics::GetGameState(this));
            if (ShooterPlayerGameState && ShooterPlayerGameState->TopScoringPlayers.Contains(ShooterPlayerState))
            {
                MulticastGainedTheLead();
            }
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
    if (ShooterPlayerGameMode && !bLeftGame)
    {
        ShooterPlayerGameMode->RequestRespawn(this, Controller);
    }
    if (bLeftGame && IsLocallyControlled())
    {
        OnLeftGame.Broadcast();
    }

}

void AShooterPlayer::ServerLeaveGame_Implementation()
{
    AShooterPlayerGameMode* ShooterPlayerGameMode = GetWorld()->GetAuthGameMode<AShooterPlayerGameMode>();
    ShooterPlayerState = ShooterPlayerState == nullptr? GetPlayerState<AShooterPlayerState>() : ShooterPlayerState;
    if (ShooterPlayerGameMode && ShooterPlayerState)
    {
        ShooterPlayerGameMode->PlayerLeftGame(ShooterPlayerState);
    }


}

void AShooterPlayer::OnRep_Health(float LastHealth)
{
    UpdateHUDHealth();
    if (CurrHealth < LastHealth) 
    {
        PlayHitReactMontage();

    }
}

void AShooterPlayer::OnRep_Shield(float LastShield)
{
    UpdateHUDShield();
    if (CurrShield < LastShield)
    {
        PlayHitReactMontage();
    }
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

bool AShooterPlayer::IsLocallyReloading()
{
    if (CombatComp == nullptr) return false;
    return CombatComp->bLocallyReloading;
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
