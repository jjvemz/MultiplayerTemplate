// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"
#include "Multiplayertest/Character/ShooterPlayer.h"
#include "Multiplayertest/Multiplayertest.h"
#include "Multiplayertest/Weapons/WeaponActor.h"

#include "Components/BoxComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

#include "DrawDebugHelpers.h"


ULagCompensationComponent::ULagCompensationComponent()
{

	PrimaryComponentTick.bCanEverTick = true;

}


void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
   
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
    ShooterCharacter = ShooterCharacter == nullptr ? Cast<AShooterPlayer>(GetOwner()) : ShooterCharacter;
    if (ShooterCharacter)
    {
        Package.Time = GetWorld()->GetTimeSeconds();
        Package.ShooterPlayer = ShooterCharacter;
        for (auto& BoxPair : ShooterCharacter->HitCollisionBoxes)
        {
            FBoxInformation BoxInformation;
            BoxInformation.Location = BoxPair.Value->GetComponentLocation();
            BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
            BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
            Package.HitBoxInfo.Add(BoxPair.Key, BoxInformation);
        }
    }

}

void ULagCompensationComponent::SaveFramePackage()
{
    if (ShooterCharacter == nullptr || !ShooterCharacter->HasAuthority()) return;
    if (FrameHistory.Num() <= 1)
    {
        FFramePackage ThisFrame;
        SaveFramePackage(ThisFrame);
        FrameHistory.AddHead(ThisFrame);
    }
    else
    {
        float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
        while (HistoryLength > MaxRecordTime)
        {
            FrameHistory.RemoveNode(FrameHistory.GetTail());
            HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
        }
        FFramePackage ThisFrame;
        SaveFramePackage(ThisFrame);
        FrameHistory.AddHead(ThisFrame);

        //Descomentar solo para el debuggeo
        ShowFramePackage(ThisFrame, FColor::Blue);
    }
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime)
{
    const float Dist = YoungerFrame.Time - OlderFrame.Time;
    const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Dist, 0.f, 1.f);

    FFramePackage InterpFramePackage;
    InterpFramePackage.Time = HitTime;

    for (auto& YoungerPair : YoungerFrame.HitBoxInfo)
    {
        const FName& BoxInfoName = YoungerPair.Key;

        const FBoxInformation& OlderBox = OlderFrame.HitBoxInfo[BoxInfoName];
        const FBoxInformation& YoungerBox = YoungerFrame.HitBoxInfo[BoxInfoName];

        FBoxInformation InterpBoxInfo;

        InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.f, InterpFraction);
        InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterpFraction);
        InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;

        InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
    }

    return InterpFramePackage;
}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package, AShooterPlayer* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
    if (HitCharacter == nullptr) return FServerSideRewindResult();
    
    FFramePackage CurrentFrame;
    CacheBoxPositions(HitCharacter, CurrentFrame);
    MoveBoxes(HitCharacter, Package);
    EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);


    //Colision para la cabeza
    UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
    HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);

    FHitResult ConfirmHitResult;
    const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
    UWorld* World = GetWorld();
    if(World)
    {
        World->LineTraceSingleByChannel(
            ConfirmHitResult,
            TraceStart,
            TraceEnd,
            ECC_HitBox
        );
        if (ConfirmHitResult.bBlockingHit) //headshot!
        {
            /*
            if (ConfirmHitResult.Component.IsValid())
            {
                UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
                if (Box)
                {
                    DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 8.f);
                }
            }*/
            ResetHitBoxes(HitCharacter, CurrentFrame);
            EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
            return FServerSideRewindResult{ true, true };
        }
        else //no fue headshot, buscará el resto de los hitboxes
        {
            for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
            {
                if (HitBoxPair.Value != nullptr)
                {
                    HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                    HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
                }
            }
            World->LineTraceSingleByChannel(
                ConfirmHitResult,
                TraceStart,
                TraceEnd,
                ECC_HitBox
            );
            if (ConfirmHitResult.bBlockingHit)
            {
                if (ConfirmHitResult.bBlockingHit)
                {
                    /*
                    UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
                    if (Box)
                    {
                        DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, 8.f);
                    }*/
                }
                ResetHitBoxes(HitCharacter, CurrentFrame);
                EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
                return FServerSideRewindResult{ true, false };
            }
        }
    }
    ResetHitBoxes(HitCharacter, CurrentFrame);
    EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
    return FServerSideRewindResult{ false, false };
}

FServerSideRewindResult ULagCompensationComponent::ProjectileConfirmHit(const FFramePackage& Package, AShooterPlayer* HitCharacter, const FVector_NetQuantize &TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
    FFramePackage CurrentFrame;
    CacheBoxPositions(HitCharacter, CurrentFrame);
    MoveBoxes(HitCharacter, Package);
    EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

    //Activar la colisión para la cabeza
    UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("Head")];
    HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);

    FPredictProjectilePathParams PathParams;
    PathParams.bTraceWithCollision = true;
    PathParams.MaxSimTime = MaxRecordTime;
    PathParams.LaunchVelocity = InitialVelocity;
    PathParams.StartLocation = TraceStart;
    PathParams.SimFrequency = 15.f;
    PathParams.ProjectileRadius =5.f;
    PathParams.TraceChannel = ECC_HitBox;
    PathParams.ActorsToIgnore.Add(GetOwner());
    PathParams.DrawDebugTime = 5.f;
    PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;

    FPredictProjectilePathResult PathRes;
    UGameplayStatics::PredictProjectilePath(this, PathParams, PathRes);

    if(PathRes.HitResult.bBlockingHit) //Le dimos a la cabeza, entramos al return de una
    {
        if(PathRes.HitResult.Component.IsValid())
        {
            UBoxComponent* Box = Cast<UBoxComponent>(PathRes.HitResult.Component);
            if(Box)
            {
                DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 8.f);
            }
        }
        ResetHitBoxes(HitCharacter, CurrentFrame);
        EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
        return FServerSideRewindResult{
            true,
            true
        };
    }
    else //No fue headshot; Hay que revisar el resto de las cajas
    {
        for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}

    UGameplayStatics::PredictProjectilePath(this, PathParams, PathRes);
    if(PathRes.HitResult.bBlockingHit)
        {
            if (PathRes.HitResult.Component.IsValid())
			{
				UBoxComponent* Box = Cast<UBoxComponent>(PathRes.HitResult.Component);
				if (Box)
				{
					DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, 8.f);
				}
			}
            ResetHitBoxes(HitCharacter, CurrentFrame);
            EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
            return FServerSideRewindResult {
                true, 
                false
            };
        }   
    }

    ResetHitBoxes(HitCharacter, CurrentFrame);
    EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
    return FServerSideRewindResult{
        false,
        false
    };
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
    for (auto& Frame : FramePackages)
    {
        if (Frame.ShooterPlayer == nullptr) return FShotgunServerSideRewindResult();
    }
    FShotgunServerSideRewindResult ShotgunResult;
    TArray<FFramePackage> CurrentFrames;
    for (auto& Frame : FramePackages)
    {
        FFramePackage CurrentFrame;
        CurrentFrame.ShooterPlayer = Frame.ShooterPlayer;
        CacheBoxPositions(Frame.ShooterPlayer, CurrentFrame);
        MoveBoxes(Frame.ShooterPlayer, Frame);
        EnableCharacterMeshCollision(Frame.ShooterPlayer, ECollisionEnabled::NoCollision);
        CurrentFrames.Add(CurrentFrame);
    }

    for (auto& Frame : FramePackages)
    {
        // Enable collision for the head first
        UBoxComponent* HeadBox = Frame.ShooterPlayer->HitCollisionBoxes[FName("head")];
        HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        HeadBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
        HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
    }


    UWorld* World = GetWorld();
    // check for head shots
    for (auto& HitLocation : HitLocations)
    {
        FHitResult ConfirmHitResult;
        const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
        if (World)
        {
            World->LineTraceSingleByChannel(
                ConfirmHitResult,
                TraceStart,
                TraceEnd,
                ECC_HitBox
            );
            AShooterPlayer* ShooterPlayer = Cast<AShooterPlayer>(ConfirmHitResult.GetActor());
            if (ShooterPlayer)
            {
                /*
                if (ConfirmHitResult.Component.IsValid())
                {
                    UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
                    if (Box)
                    {
                        DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 8.f);
                    }
                }*/
                if (ShotgunResult.HeadShots.Contains(ShooterPlayer))
                {
                    ShotgunResult.HeadShots[ShooterPlayer]++;
                }
                else
                {
                    ShotgunResult.HeadShots.Emplace(ShooterPlayer, 1);
                }
            }
        }
    }

    // enable collision for all boxes, then disable for head box
    for (auto& Frame : FramePackages)
    {
        for (auto& HitBoxPair : Frame.ShooterPlayer->HitCollisionBoxes)
        {
            if (HitBoxPair.Value != nullptr)
            {
                HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                HitBoxPair.Value->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
                HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
            }
        }
        UBoxComponent* HeadBox = Frame.ShooterPlayer->HitCollisionBoxes[FName("head")];
        HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // check for body shots
    for (auto& HitLocation : HitLocations)
    {
        FHitResult ConfirmHitResult;
        const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
        if (World)
        {
            World->LineTraceSingleByChannel(
                ConfirmHitResult,
                TraceStart,
                TraceEnd,
                ECC_HitBox
            );
            AShooterPlayer* ShooterPlayer = Cast<AShooterPlayer>(ConfirmHitResult.GetActor());
            if (ShooterPlayer)
            {
                if (ConfirmHitResult.Component.IsValid())
                {
                    /*
                    UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
                    if (Box)
                    {
                        DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, 8.f);
                    }*/
                }
                if (ShotgunResult.BodyShots.Contains(ShooterPlayer))
                {
                    ShotgunResult.BodyShots[ShooterPlayer]++;
                }
                else
                {
                    ShotgunResult.BodyShots.Emplace(ShooterPlayer, 1);
                }
            }
        }
    }

    for (auto& Frame : CurrentFrames)
    {
        ResetHitBoxes(Frame.ShooterPlayer, Frame);
        EnableCharacterMeshCollision(Frame.ShooterPlayer, ECollisionEnabled::QueryAndPhysics);
    }

    return ShotgunResult;
}


void ULagCompensationComponent::CacheBoxPositions(AShooterPlayer* HitCharacter, FFramePackage& OutFramePackage)
{
    if (HitCharacter == nullptr) return;

    for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
    {
        if (HitBoxPair.Value != nullptr) 
        {
            FBoxInformation BoxInfo;
            BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
            BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
            BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();

            OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
        }
    }
}

void ULagCompensationComponent::MoveBoxes(AShooterPlayer* HitCharacter, const FFramePackage& Package)
{
    if (HitCharacter == nullptr) return;

    for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
    {
        if (HitBoxPair.Value != nullptr) 
        {
            HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
            HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
            HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
        }
    }
}

void ULagCompensationComponent::ResetHitBoxes(AShooterPlayer* HitCharacter, const FFramePackage& Package)
{
    if (HitCharacter == nullptr) return;
    for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
    {
        if (HitBoxPair.Value != nullptr)
        {
            HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
            HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
            HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
            HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
}

void ULagCompensationComponent::EnableCharacterMeshCollision(AShooterPlayer* HitCharacter, ECollisionEnabled::Type CollisionEnabled)
{
    if (HitCharacter && HitCharacter->GetMesh())
    {
        HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
    }
}

 void ULagCompensationComponent::ProjectileServerScoreRequest_Implementation(AShooterPlayer* HitCharacter,const FVector_NetQuantize& TraceStart,const FVector_NetQuantize100& InitialVelocity,float HitTime)
 {
    FServerSideRewindResult Confirm = ProjectileServerSideRewind(HitCharacter, TraceStart, InitialVelocity, HitTime);

     if (ShooterCharacter && HitCharacter  && Confirm.bHitConfirmed && ShooterCharacter->GetEquippedWeapon())
    {
         const float Damage = Confirm.bHeadShot ? ShooterCharacter->GetEquippedWeapon()->GetHeadShotDamage() : ShooterCharacter->GetEquippedWeapon()->GetDamage();
        UGameplayStatics::ApplyDamage(
            HitCharacter,
            Damage,
            ShooterCharacter->Controller,
            ShooterCharacter->GetEquippedWeapon(),
            UDamageType::StaticClass()
        );
    }

 }

void ULagCompensationComponent::ServerScoreRequest_Implementation(AShooterPlayer* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
    FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);
    
    if (ShooterCharacter && HitCharacter && ShooterCharacter->GetEquippedWeapon() && Confirm.bHitConfirmed)
    {
        const float Damage = Confirm.bHeadShot ? ShooterCharacter->GetEquippedWeapon()->GetHeadShotDamage() : ShooterCharacter->GetEquippedWeapon()->GetDamage();
        UGameplayStatics::ApplyDamage(
            HitCharacter,
            Damage,
            ShooterCharacter->Controller,
            ShooterCharacter->GetEquippedWeapon(),
            UDamageType::StaticClass()
        );
    }
}

void ULagCompensationComponent::ShotgunServerScoreRequest_Implementation(const TArray<AShooterPlayer*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
    FShotgunServerSideRewindResult Confirm = ShotgunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);

    for (auto& HitCharacter : HitCharacters)
    {
        if (HitCharacter == nullptr || HitCharacter->GetEquippedWeapon() == nullptr || ShooterCharacter == nullptr) continue;

        float TotalDamage = 0.f;

        if (Confirm.HeadShots.Contains(HitCharacter))
        {
            float HeadShotDamage = Confirm.HeadShots[HitCharacter] * HitCharacter->GetEquippedWeapon()->GetHeadShotDamage();
            TotalDamage += HeadShotDamage;
        }
        if (Confirm.BodyShots.Contains(HitCharacter))
        {
            float BodyShotDamage = Confirm.BodyShots[HitCharacter] * HitCharacter->GetEquippedWeapon()->GetDamage();
            TotalDamage += BodyShotDamage;
        }

        UGameplayStatics::ApplyDamage(
            HitCharacter,
            TotalDamage,
            ShooterCharacter->Controller,
            HitCharacter->GetEquippedWeapon(),
            UDamageType::StaticClass()
        );
    }
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)
{
    for (auto& BoxInfo : Package.HitBoxInfo)
    {
        /*
        DrawDebugBox(
            GetWorld(),
            BoxInfo.Value.Location,
            BoxInfo.Value.BoxExtent,
            FQuat(BoxInfo.Value.Rotation),
            Color,
            false,
            4.f
        );*/
    }
}

FFramePackage ULagCompensationComponent::GetFrameToCheck(AShooterPlayer* HitCharacter, float HitTime)
{
    bool bReturn = HitCharacter == nullptr
        || HitCharacter->GetLagCompensation() == nullptr
        || HitCharacter->GetLagCompensation()->FrameHistory.GetHead() == nullptr
        || HitCharacter->GetLagCompensation()->FrameHistory.GetTail() == nullptr;

    if (bReturn) return FFramePackage();

    //FramePackage para revisar si fue un Hit
    FFramePackage FrameToCheck;
    bool bShouldInterpolate = true;

    // Frame history del HitCharacter
    const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensation()->FrameHistory;
    const float OldestHistoryTime = History.GetTail()->GetValue().Time;
    const float NewestHistoryTime = History.GetHead()->GetValue().Time;


    if (OldestHistoryTime > HitTime)
    {
        //Muy laggeado para el SSR
        return FFramePackage();
    }
    if (OldestHistoryTime == HitTime)
    {
        FrameToCheck = History.GetTail()->GetValue();
        bShouldInterpolate = false;

    }

    if (NewestHistoryTime <= HitTime)
    {
        FrameToCheck = History.GetHead()->GetValue();
        bShouldInterpolate = false;
    }


    TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
    TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;

    while (Older->GetValue().Time > HitTime)
    {
        //March back hasta que OlderTime< HitTime>YoungerTime
        if (Older->GetNextNode() == nullptr) break;
        Older = Older->GetNextNode();

        if (Older->GetValue().Time > HitTime)
        {
            Younger = Older;
        }
    }

    if (Older->GetValue().Time == HitTime) // Poco probable, pero encontramos al frame a revisar!
    {
        FrameToCheck = Older->GetValue();
        bShouldInterpolate = false;
    }

    if (bShouldInterpolate)
    {
        // Interpolar entre Younger y Older
        FrameToCheck = InterpBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
    }
    FrameToCheck.ShooterPlayer = HitCharacter;
    return FrameToCheck;
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(AShooterPlayer* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{

    FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
    return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

FServerSideRewindResult ULagCompensationComponent::ProjectileServerSideRewind(AShooterPlayer *HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{

    FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
    return ProjectileConfirmHit(FrameToCheck, HitCharacter, TraceStart, InitialVelocity, HitTime);
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunServerSideRewind(const TArray<AShooterPlayer *> &HitCharacters, const FVector_NetQuantize &TraceStart, const TArray<FVector_NetQuantize> &HitLocations, float HitTime)
{
    TArray<FFramePackage> FramesToCheck;

    for (AShooterPlayer* HitCharacter : HitCharacters)
    {
        FramesToCheck.Add(GetFrameToCheck(HitCharacter, HitTime));
    }
    return FShotgunServerSideRewindResult();
}



void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    SaveFramePackage();
}



