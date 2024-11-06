


#include "WeaponActor.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

#include "Multiplayertest/Character/ShooterPlayer.h"
#include "Multiplayertest/PlayerController/ShooterPlayerController.h"
#include "Multiplayertest/Components/CombatComponent.h"

#include "Animation/AnimationAsset.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"


AWeaponActor::AWeaponActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	EnableCustomDepth(true);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeaponActor::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

void AWeaponActor::BeginPlay()
{
	Super::BeginPlay();

    AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeaponActor::OnSphereOverlap);
    AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeaponActor::OnSphereEndOverlap);

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
    if (!HasAuthority())
    {
        FireDelay = 0.001f;
    }
}

void AWeaponActor::OnWeaponStateSet()
{
    switch (WeaponState)
    {
    case EWeaponState::EWS_Equipped:
        OnEquipped();
        break;

    case EWeaponState::EWS_EquippedSecondary:
        OnEquippedSecondary();
        break;
    case EWeaponState::EWS_Dropped:
        OnDropped();
        break;
    }
}

void AWeaponActor::OnEquipped()
{
    ShowPickupWidget(false);
    AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponMesh->SetSimulatePhysics(false);
    WeaponMesh->SetEnableGravity(false);
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (WeaponType == EWeaponType::EWT_SubmachineGun)
    {
        WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        WeaponMesh->SetEnableGravity(true);
        WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    }
    EnableCustomDepth(false);
    ShooterCharacter = ShooterCharacter == nullptr ? Cast<AShooterPlayer>(GetOwner()) : ShooterCharacter;
    if (ShooterCharacter && bUseServerSideRewind)
    {
        ShooterOwnerController = ShooterOwnerController == nullptr ? Cast<AShooterPlayerController>(ShooterCharacter->Controller) : ShooterOwnerController;
        if (ShooterOwnerController && HasAuthority() && !ShooterOwnerController->HighPingDelegate.IsBound())
        {
            ShooterOwnerController->HighPingDelegate.AddDynamic(this, &AWeaponActor::OnPingTooHigh);
        }
    }
}

void AWeaponActor::OnDropped()
{
    if (HasAuthority())
    {
        AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }
    WeaponMesh->SetSimulatePhysics(true);
    WeaponMesh->SetEnableGravity(true);
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
    WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
    WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
    WeaponMesh->MarkRenderStateDirty();
    EnableCustomDepth(true);

    ShooterCharacter = ShooterCharacter == nullptr ? Cast<AShooterPlayer>(GetOwner()) : ShooterCharacter;
    if (ShooterCharacter)
    {
        ShooterOwnerController = ShooterOwnerController == nullptr ? Cast<AShooterPlayerController>(ShooterCharacter->Controller) : ShooterOwnerController;
        if (ShooterOwnerController && HasAuthority() && ShooterOwnerController->HighPingDelegate.IsBound())
        {
            ShooterOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeaponActor::OnPingTooHigh);
        }
    }
}

void AWeaponActor::OnEquippedSecondary()
{
    ShowPickupWidget(false);
    AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponMesh->SetSimulatePhysics(false);
    WeaponMesh->SetEnableGravity(false);
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (WeaponType == EWeaponType::EWT_SubmachineGun)
    {
        WeaponMesh->SetEnableGravity(true);
        WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    }
    if (WeaponMesh)
    {
        WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
        WeaponMesh->MarkRenderStateDirty();
    }

}

void AWeaponActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeaponActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeaponActor, WeaponState);
    DOREPLIFETIME_CONDITION(AWeaponActor, bUseServerSideRewind, COND_OwnerOnly);

}


void AWeaponActor::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AShooterPlayer* ShooterPlayer = Cast<AShooterPlayer>(OtherActor);
	if (ShooterPlayer)
	{
		ShooterPlayer->SetOverlappingWeapon(this);
	}
}

void AWeaponActor::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AShooterPlayer* ShooterPlayer = Cast<AShooterPlayer>(OtherActor);
	if (ShooterPlayer)
	{
		ShooterPlayer->SetOverlappingWeapon(nullptr);
	}
}

void AWeaponActor::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (Owner == nullptr)
	{
		ShooterCharacter = NULL;
		ShooterOwnerController = NULL;
	}
	else
	{
        ShooterCharacter = ShooterCharacter == nullptr ? Cast<AShooterPlayer>(Owner) : ShooterCharacter;
        if (ShooterCharacter && ShooterCharacter->GetEquippedWeapon() 
            && ShooterCharacter->GetEquippedWeapon() == this)
        {
            SetHUDAmmo();

        }
	}
}

/*
void AWeaponActor::OnRep_Ammo()
{
    ShooterCharacter = ShooterCharacter == nullptr ? Cast<AShooterPlayer>(Owner) : ShooterCharacter;
	SetHUDAmmo();

	if (ShooterCharacter && ShooterCharacter->GetCombat() && IsFull())
	{
        ShooterCharacter->GetCombat()->JumpToShotgunEnd();
	}
}
*/

void AWeaponActor::SetHUDAmmo()
{
	ShooterCharacter = ShooterCharacter == nullptr ? Cast<AShooterPlayer>(GetOwner()) : ShooterCharacter;
	if (ShooterCharacter)
	{
		ShooterOwnerController = ShooterOwnerController == nullptr ? Cast<AShooterPlayerController>(ShooterCharacter->Controller) : ShooterOwnerController;
		if (ShooterOwnerController)
		{
			ShooterOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeaponActor::AddAmmo(int32 AmmoToAdd)
{
    Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
    SetHUDAmmo();
    ClientAddAmmo(AmmoToAdd);
}

void AWeaponActor::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();
    if (HasAuthority())
    {
        ClientUpdateAmmo(Ammo);
    }
    else
    {
        ++Sequence;
    }
}

void AWeaponActor::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
    if (HasAuthority()) return;
    Ammo = ServerAmmo;
    --Sequence;
    Ammo -= Sequence;
    SetHUDAmmo();
}

void AWeaponActor::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
    if (HasAuthority()) return;
    Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
    ShooterCharacter = ShooterCharacter == nullptr ? Cast<AShooterPlayer>(GetOwner()) : ShooterCharacter;
    if (ShooterCharacter && ShooterCharacter->GetCombat() && IsFull()) 
    {
        ShooterCharacter->GetCombat()->JumpToShotgunEnd();
    }
    SetHUDAmmo();
}


void AWeaponActor::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);

		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if (WeaponType == EWeaponType::EWT_SubmachineGun)
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		}

		break;
	case EWeaponState::EWS_Dropped:
		if (HasAuthority())
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

		WeaponMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
		WeaponMesh->MarkRenderStateDirty();
		EnableCustomDepth(true);

		break;
	}
}

bool AWeaponActor::IsEmpty()
{
	return Ammo <= 0;
}

bool AWeaponActor::IsFull()
{
	return Ammo ==MagCapacity;
}

FVector AWeaponActor::TraceEndWithScatter(const FVector& HitTarget)
{
    const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
    if (MuzzleFlashSocket == nullptr) return FVector();

    const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
    const FVector TraceStart = SocketTransform.GetLocation();


    const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
    const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
    const FVector RandVector = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
    const FVector EndLocation = SphereCenter + RandVector;
    const FVector ToEndLocaction = EndLocation - TraceStart;
    /*
    DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
    DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
    DrawDebugLine(
        GetWorld(),
        TraceStart,
        FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()),
        FColor::Cyan,
        true);*/
    return FVector(TraceStart + ToEndLocaction * TRACE_LENGTH / ToEndLocaction.Size());
}

void AWeaponActor::OnPingTooHigh(bool bPingTooHigh)
{
    bUseServerSideRewind = !bPingTooHigh;
}

void AWeaponActor::OnRep_WeaponState()
{
    OnWeaponStateSet();
}



void AWeaponActor::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeaponActor::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
    if (HasAuthority())
    {
        SpendRound();
    }
}

void AWeaponActor::DroppedWeapon()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(NULL);
	ShooterCharacter = nullptr;
	ShooterOwnerController = nullptr;

}

