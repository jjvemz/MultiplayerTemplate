// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Multiplayertest/Character/ShooterPlayer.h"
#include "Multiplayertest/Character/ShooterPlayerAnimInstance.h"
#include "Multiplayertest/Weapons/WeaponActor.h"
#include "Multiplayertest/PlayerController/ShooterPlayerController.h"
#include "Multiplayertest/Weapons/Shotgun.h"
//#include "Multiplayertest/HUD/ShooterHUD.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"

UCombatComponent::UCombatComponent()
{

    PrimaryComponentTick.bCanEverTick = true;
    BaseWalkSpeed = 600.f;
    AimWalkSpeed = 450.f;
}


void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UCombatComponent, EquippedWeapon);
    DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
    DOREPLIFETIME(UCombatComponent, bAiming);
    DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
    DOREPLIFETIME(UCombatComponent, CombatState);
    DOREPLIFETIME(UCombatComponent, Grenades);

}

void UCombatComponent::BeginPlay()
{
    Super::BeginPlay();
    if (Character)
    {
        Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
        if (Character->GetFollowCamera())
        {
            DefaultFOV = Character->GetFollowCamera()->FieldOfView;
            CurrentFOV = DefaultFOV;

        }
        if (Character->HasAuthority())
        {
            InitializeCarriedAmmo();
        }
    }

}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (Character && Character->IsLocallyControlled())
    {

        FHitResult HitResult;
        TraceUnderCrosshair(HitResult);
        HitTarget = HitResult.ImpactPoint;

        SetHUDCrosshairs(DeltaTime);
        InterpFOV(DeltaTime);

    }
}

void UCombatComponent::Fire()
{

    if (CanFire())
    {
        bCanFire = false;

        if (EquippedWeapon)
        {
            //bCanFire = false;
            CrosshairShootingFactor = 0.75f;

            switch (EquippedWeapon->FireType) 
            {
            case EFireType::EFT_Projectile:
                FireProjectileWeapon();
                break;
            case EFireType::EFT_HitScan:
                FireHitScanWeapon();
                break;
            case EFireType::EFT_Shotgun:
                FireShotgun();
                break;
            }
        }
        StartFireTimer();
    }

}

void UCombatComponent::FireProjectileWeapon()
{
    if (EquippedWeapon && Character)
    {
        HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
        
        if (!Character->HasAuthority()) LocalFire(HitTarget);
        
        ServerFire(HitTarget);

    }
}

void UCombatComponent::FireHitScanWeapon()
{
    if (EquippedWeapon && Character)
    {
        HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
        if (!Character->HasAuthority()) LocalFire(HitTarget);
        ServerFire(HitTarget);
    }
}

void UCombatComponent::FireShotgun()
{
    AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
    if (Shotgun)
    {
        TArray<FVector_NetQuantize> HitTargets;
        Shotgun->ShotgunTraceEndWithScatter(HitTarget, HitTargets);

        if (!Character->HasAuthority()) ShotgunLocalFire(HitTargets);
        ServerShotgunFire(HitTargets);
    }
}

void UCombatComponent::ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
    AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
    if (Shotgun == nullptr || Character == nullptr) return;
    if (CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Unoccupied)
    {
        Character->PlayShootingMontage(bAiming);
        Shotgun->FireShotgun(TraceHitTargets);
        CombatState = ECombatState::ECS_Unoccupied;
    }
}

void UCombatComponent::StartFireTimer()
{
    if (EquippedWeapon == nullptr || Character == nullptr) return;

    Character->GetWorldTimerManager().SetTimer(
        FireTimer, this, &UCombatComponent::FireTimerFinished, EquippedWeapon->FireDelay
    );
}

void UCombatComponent::FireTimerFinished()
{
    if (EquippedWeapon == nullptr) return;
    bCanFire = true;
    if (bFirePressed && EquippedWeapon->bAutomatic)
    {
        Fire();
    }
    ReloadEmptyWeapon();
}

void UCombatComponent::Reload()
{
    if (CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied 
        && EquippedWeapon && !EquippedWeapon->IsFull() && !bLocallyReloading)
    {
        ServerReload();
        HandleReload();
        bLocallyReloading = true;
    }
}

void UCombatComponent::ServerReload_Implementation()
{
    if (Character == nullptr || EquippedWeapon == nullptr) return;
    CombatState = ECombatState::ECS_Reloading;
    if(!Character->IsLocallyControlled()) HandleReload();
}


void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
    MulticastFire(TraceHitTarget);
}


void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
    if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
    LocalFire(TraceHitTarget);
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
    MulticastShotgunFire(TraceHitTargets);
}

void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
    if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
    ShotgunLocalFire(TraceHitTargets);
}


void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
    if (EquippedWeapon == nullptr) return;

    if (Character && CombatState == ECombatState::ECS_Unoccupied)
    {
        Character->PlayShootingMontage(bAiming);
        EquippedWeapon->Fire(TraceHitTarget);
    }
}

void UCombatComponent::EquipWeapon(AWeaponActor* WeaponToEquip)
{
    if (Character == nullptr || WeaponToEquip == nullptr) return;

    if (CombatState != ECombatState::ECS_Unoccupied) return;

    if (EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
    {
        EquipSecondaryWeapon(WeaponToEquip);
    }
    else
    {
        EquipPrimaryWeapon(WeaponToEquip);
    }

    Character->GetCharacterMovement()->bOrientRotationToMovement = false;
    Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::SwapWeapons()
{
    if (CombatState != ECombatState::ECS_Unoccupied) return;
    AWeaponActor* TempWeapon = EquippedWeapon;
    EquippedWeapon = SecondaryWeapon;
    SecondaryWeapon = TempWeapon;

    EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
    AttachActorToRightHand(EquippedWeapon);
    EquippedWeapon->SetHUDAmmo();
    UpdateCarriedAmmo();
    PlayEquipWeaponSound(EquippedWeapon);

    SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
    AttachActorToBackpack(SecondaryWeapon);
}


void UCombatComponent::FinishReloading()
{
    if (Character == nullptr) return;
    bLocallyReloading = false;
    if (Character->HasAuthority())
    {
        CombatState = ECombatState::ECS_Unoccupied;
        UpdateAmmoValues();
    }
    if (bFirePressed)
    {
        Fire();
    }
}

void UCombatComponent::UpdateAmmoValues()
{
    if (Character == nullptr || EquippedWeapon == nullptr) return;
    int32 ReloadAmount = AmountToReload();
    if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
    {
        CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
        CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
    }
    Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
    if (Controller)
    {
        Controller->SetHUDcarriedAmmo(CarriedAmmo);
    }
    EquippedWeapon->AddAmmo(ReloadAmount);
}

void UCombatComponent::UpdateShotgunAmmoValues()
{

    if (Character == nullptr || EquippedWeapon == nullptr) return;

    if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
    {
        CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
        CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
    }
    Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
    if (Controller)
    {
        Controller->SetHUDcarriedAmmo(CarriedAmmo);
    }
    EquippedWeapon->AddAmmo(1);
    bCanFire = true;
    if (EquippedWeapon->IsFull() || CarriedAmmo == 0)
    {
        JumpToShotgunEnd();
    }
}

void UCombatComponent::OnRep_CombatState()
{
    switch (CombatState)
    {
    case ECombatState::ECS_Reloading:
        if (Character && !Character->IsLocallyControlled()) HandleReload();
        break;
    case ECombatState::ECS_Unoccupied:
        if (bFirePressed)
        {
            Fire();
        }
        break;
    case ECombatState::ECS_ThrowingGrenade:
        if (Character && !Character->IsLocallyControlled())
        {
            Character->PlayThrowGrenadeMontage();
            AttachActorToLeftHand(EquippedWeapon);
            ShowAttachedGrenade(true);
        }
        break;
    }
}

void UCombatComponent::HandleReload()
{
    if (Character)
    {
        Character->PlayReloadMontage();
    }
}

int32 UCombatComponent::AmountToReload()
{
    if (EquippedWeapon == nullptr) return 0;
    int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();

    if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
    {
        int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
        int32 Least = FMath::Min(RoomInMag, AmountCarried);
        return FMath::Clamp(RoomInMag, 0, Least);
    }
    return 0;
}

void UCombatComponent::ThrowGrenade()
{
    if (Grenades == 0) return;

    if (CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr) return;
    CombatState = ECombatState::ECS_ThrowingGrenade;
    if (Character)
    {
        Character->PlayThrowGrenadeMontage();
        AttachActorToLeftHand(EquippedWeapon);
        ShowAttachedGrenade(true);
    }
    if (Character && !Character->HasAuthority())
    {
        ServerThrowGrenade();
    }
    if (Character && Character->HasAuthority())
    {
        Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
        UpdateHUDGrenades();
    }
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{

    if (Grenades == 0) return;
    CombatState = ECombatState::ECS_ThrowingGrenade;

    if (Character)
    {
        Character->PlayThrowGrenadeMontage();
        AttachActorToLeftHand(EquippedWeapon);
        ShowAttachedGrenade(true);
    }
}

void UCombatComponent::OnRep_EquippedWeapon()
{
    if (EquippedWeapon && Character)
    {
        EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
        AttachActorToRightHand(EquippedWeapon);

        Character->GetCharacterMovement()->bOrientRotationToMovement = false;
        Character->bUseControllerRotationYaw = true;

        PlayEquipWeaponSound(EquippedWeapon);

        EquippedWeapon->EnableCustomDepth(false);
        EquippedWeapon->SetHUDAmmo();
    }
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
    if (SecondaryWeapon && Character)
    {
        SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
        AttachActorToBackpack(SecondaryWeapon);

    }
}


void UCombatComponent::TraceUnderCrosshair(FHitResult& TraceHitResult)
{
    FVector2D ViewportSize;

    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->GetViewportSize(ViewportSize);
    }
    FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
    FVector CrosshairWorldPosition;
    FVector CrosshairWorldDirection;
    bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
        UGameplayStatics::GetPlayerController(this, 0),
        CrosshairLocation,
        CrosshairWorldPosition,
        CrosshairWorldDirection
    );

    if (bScreenToWorld)
    {
        FVector Start = CrosshairWorldPosition;

        if (Character)
        {
            float DistanceToTheCharacter = (Character->GetActorLocation() - Start).Size();
            Start += CrosshairWorldDirection * (DistanceToTheCharacter + 100.f);

        }

        FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

        GetWorld()->LineTraceSingleByChannel(
            TraceHitResult,
            Start,
            End,
            ECollisionChannel::ECC_Visibility
        );

        if (!TraceHitResult.bBlockingHit)
        {
            TraceHitResult.ImpactPoint = End;

        }

        if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractCrosshairsInterface>())
        {
            HUDPackage.CrosshairColor = FLinearColor::Red;
        }
        else
        {
            HUDPackage.CrosshairColor = FLinearColor::White;

        }

    }
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
    if (Character == nullptr || Character->Controller == nullptr) return;

    Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;

    if (Controller)
    {
        HUD = HUD == nullptr ? Cast<AShooterHUD>(Controller->GetHUD()) : HUD;
        if (HUD)
        {

            if (EquippedWeapon) {
                HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
                HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
                HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
                HUDPackage.CrosshairsUp = EquippedWeapon->CrosshairsUp;
                HUDPackage.CrosshairsDown = EquippedWeapon->CrosshairsDown;
            }
            else
            {
                HUDPackage.CrosshairsCenter = nullptr;
                HUDPackage.CrosshairsLeft = nullptr;
                HUDPackage.CrosshairsRight = nullptr;
                HUDPackage.CrosshairsUp = nullptr;
                HUDPackage.CrosshairsDown = nullptr;
            }
            //Calcula el spread de la mira
            FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
            FVector2D VelocityMultiplierRange(0.f, 1.f);
            FVector Velocity = Character->GetVelocity();
            Velocity.Z = 0.f;

            CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

            if (Character->GetCharacterMovement()->IsFalling())
            {
                CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.35f, DeltaTime, 2.35f);

            }
            else
            {
                CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 0.f);

            }

            if (bAiming)
            {
                CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, InterpToCrossHairAimFactor, DeltaTime, 30.f);
            }
            else
            {
                CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);

            }
            CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);

            HUDPackage.CrosshairSpread = CrosshairVelocityFactor + CrosshairInAirFactor
                + CrosshairCrouchedFactor + CrosshairAimFactor + CrosshairShootingFactor;

            HUD->SetHUDPackage(HUDPackage);

        }
    }
}

void UCombatComponent::DropEquippedWeapon()
{
    if (EquippedWeapon) EquippedWeapon->DroppedWeapon();
}

void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
    if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr || EquippedWeapon == nullptr) return;

    bool bUsePistolSocket =
        EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol ||
        EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SubmachineGun;

    FName SocketName = bUsePistolSocket ? FName("PistolSocket") : FName("LeftHandSocket");
    const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(SocketName);

    if (HandSocket)
    {
        HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
    }
}

void UCombatComponent::AttachActorToBackpack(AActor* ActorToAttach)
{
    if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
    const USkeletalMeshSocket* BackpackSocket = Character->GetMesh()->GetSocketByName(FName("BackpackSocket"));
    
    if (BackpackSocket) BackpackSocket->AttachActor(ActorToAttach, Character->GetMesh());
}

void UCombatComponent::UpdateCarriedAmmo()
{
    if (EquippedWeapon == nullptr) return;
    if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
    {
        CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
    }

    Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
    if (Controller)
    {
        Controller->SetHUDcarriedAmmo(CarriedAmmo);
    }
}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
    if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
    const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));

    if (HandSocket)
    {
        HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
    }
}


void UCombatComponent::PlayEquipWeaponSound(AWeaponActor* WeaponToEquip)
{
    if (Character && WeaponToEquip && WeaponToEquip->EquipSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            WeaponToEquip->EquipSound,
            Character->GetActorLocation()
        );
    }
}

void UCombatComponent::EquipPrimaryWeapon(AWeaponActor* WeaponToEquip)
{
    if (WeaponToEquip == nullptr) return;
    DropEquippedWeapon();
    EquippedWeapon = WeaponToEquip;
    EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
    AttachActorToRightHand(EquippedWeapon);
    EquippedWeapon->SetOwner(Character);
    EquippedWeapon->SetHUDAmmo();
    UpdateCarriedAmmo();
    PlayEquipWeaponSound(WeaponToEquip);
    ReloadEmptyWeapon();
}

void UCombatComponent::EquipSecondaryWeapon(AWeaponActor* WeaponToEquip)
{
    if (WeaponToEquip == nullptr) return;
    SecondaryWeapon = WeaponToEquip;
    SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
    AttachActorToBackpack(WeaponToEquip);
    PlayEquipWeaponSound(WeaponToEquip);
    
    SecondaryWeapon->SetOwner(Character);
}

void UCombatComponent::ReloadEmptyWeapon()
{
    if (EquippedWeapon && EquippedWeapon->IsEmpty())
    {
        Reload();
    }
}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
    if (Character && Character->GetAttachedGrenade())
    {
        Character->GetAttachedGrenade()->SetVisibility(bShowGrenade);
    }
}

void UCombatComponent::OnRep_Aiming()
{
    if (Character && Character->IsLocallyControlled())
    {
        bAiming = bAimButtonPressed;
    }
}


void UCombatComponent::SetAiming(bool bIsAiming)
{
    if (Character == nullptr || EquippedWeapon == nullptr) return;
    bAiming = bIsAiming;
    ServerSetAiming(bIsAiming);

    if (Character)
    {
        Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
    }

    if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
    {
        Character->ShowSniperScopeWidget(bIsAiming);
    }
    if (Character->IsLocallyControlled()) bAimButtonPressed = bIsAiming;
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
    bAiming = bIsAiming;
    if (Character)
    {
        Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
    }
}



void UCombatComponent::FireButtonPressed(bool bPressed)
{
    bFirePressed = bPressed;
    if (bFirePressed && EquippedWeapon)
    {
        FHitResult HitResult;
        TraceUnderCrosshair(HitResult);
        //ServerFire(HitResult.ImpactPoint)
        Fire();
    }
}

void UCombatComponent::ShotgunShellReload()
{
    if (Character && Character->HasAuthority())
    {
        UpdateShotgunAmmoValues();
    }
}

void UCombatComponent::JumpToShotgunEnd()
{
    UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
    if (AnimInstance && Character->GetReloadMontage())
    {
        AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
    }

}

void UCombatComponent::ThrowGrenadeFinished()
{
    CombatState = ECombatState::ECS_Unoccupied;
    AttachActorToRightHand(EquippedWeapon);
}

void UCombatComponent::LaunchGrenade()
{
    ShowAttachedGrenade(false);
    if (Character && Character->IsLocallyControlled())
    {
        ServerLaunchGrenade(HitTarget);
    }
}

void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
    if(Character && GrenadeClass && Character->GetAttachedGrenade())
    {
        const FVector StartingLocation = Character->GetAttachedGrenade()->GetComponentLocation();
        FVector ToTarget = Target - StartingLocation;
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = Character;
        SpawnParams.Instigator = Character;
        UWorld* World = GetWorld();
        if (World)
        {
            World->SpawnActor<AProjectile>(
                GrenadeClass,
                StartingLocation,
                ToTarget.Rotation(),
                SpawnParams
            );
        }
    }
 
}

void UCombatComponent::PickUpAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{

    if (CarriedAmmoMap.Contains(WeaponType))
    {
        CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmount, 0, MaxCarriedAmmo);
        UpdateCarriedAmmo();
    }
    if (EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
    {
        Reload();
    }
}


void UCombatComponent::InterpFOV(float DeltaTime)
{
    if (EquippedWeapon == nullptr) return;

    if (bAiming)
    {
        CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomedInterpSpeed());
    }
    else
    {
        CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomedInterpSpeed);
    }
    if (Character && Character->GetFollowCamera())
    {
        Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
    }
}
bool UCombatComponent::CanFire()
{
    if (!EquippedWeapon)
    {
        return false;
    }
    if (bLocallyReloading)
    {
        // Se puede disparar la escopeta al recargar
        if (!EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun)
        {
            bLocallyReloading = false;
            return true;
        }
        return false;
    }
    return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::OnRep_CarriedAmmo()
{
    Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
    if (Controller)
    {
        Controller->SetHUDcarriedAmmo(CarriedAmmo);
    }

    bool bJumpToShotgunEnd =
        CombatState == ECombatState::ECS_Reloading
        && EquippedWeapon != NULL
        && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun
        && CarriedAmmo == 0;

    if (bJumpToShotgunEnd)
    {
        JumpToShotgunEnd();
    }
}

void UCombatComponent::InitializeCarriedAmmo()
{
    CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
    CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
    CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
    CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, StartingSMGAmmo);
    CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
    CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingShotgunAmmo);
    CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo);

}

void UCombatComponent::OnRep_Grenades()
{
    UpdateHUDGrenades();
}

void UCombatComponent::UpdateHUDGrenades()
{
    Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
    if (Controller)
    {
        Controller->SetHUDGrenades(Grenades);
    }
}

bool UCombatComponent::ShouldSwapWeapons()
{
    return (EquippedWeapon != nullptr && SecondaryWeapon != nullptr);
}
