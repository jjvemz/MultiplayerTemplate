// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#include "Multiplayertest/Character/ShooterPlayer.h"
#include "Multiplayertest/PlayerController/ShooterPlayerController.h"
#include "Multiplayertest/Components/LagCompensationComponent.h"

#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"


void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeaponActor::Fire(FVector());
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
        const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
        const FVector Start = SocketTransform.GetLocation();

		TMap<AShooterPlayer*, uint32> HitMap;
		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			AShooterPlayer* ShooterPlayer = Cast<AShooterPlayer>(FireHit.GetActor());
			if (ShooterPlayer)
			{
				if (HitMap.Contains(ShooterPlayer))
				{
					HitMap[ShooterPlayer]++;
				}
				else
				{
					HitMap.Emplace(ShooterPlayer, 1);
				}

                if (ImpactParticles)
                {
                    UGameplayStatics::SpawnEmitterAtLocation(
                        GetWorld(),
                        ImpactParticles,
                        FireHit.ImpactPoint,
                        FireHit.ImpactNormal.Rotation()
                    );
                }
                if (HitSound)
                {
                    UGameplayStatics::PlaySoundAtLocation(
                        this,
                        HitSound,
                        FireHit.ImpactPoint,
                        .5f,
                        FMath::FRandRange(-.5f, .5f)
                    );
                }
			}
			
		}
        TArray<AShooterPlayer*> HitCharacters;
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key && InstigatorController)
			{
                bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();

                if (HasAuthority() && bCauseAuthDamage)
                {
                    UGameplayStatics::ApplyDamage(
                        HitPair.Key,
                        Damage * HitPair.Value,
                        InstigatorController,
                        this,
                        UDamageType::StaticClass()
                    );
                }
                HitCharacters.Add(HitPair.Key);
			}
		}

        if (!HasAuthority() && bUseServerSideRewind)
        {
            ShooterCharacter = ShooterCharacter == nullptr ? Cast<AShooterPlayer>(OwnerPawn) : ShooterCharacter;
            ShooterOwnerController = ShooterOwnerController == nullptr ? Cast<AShooterPlayerController>(InstigatorController) : ShooterOwnerController;
            if (ShooterOwnerController && ShooterCharacter && 
                ShooterCharacter->GetLagCompensation() && ShooterCharacter->IsLocallyControlled() && ShooterCharacter->IsLocallyControlled())
            {
                ShooterCharacter->GetLagCompensation()->ShotgunServerScoreRequest(
                    HitCharacters, 
                    Start,
                    HitTargets,
                    ShooterOwnerController->GetServerTime() - ShooterOwnerController->SingleTripTime
                    );
            }
        }
	}
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
    const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
    if (MuzzleFlashSocket == nullptr) return;

    const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
    const FVector TraceStart = SocketTransform.GetLocation();

    const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
    const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;

    for (uint32 i = 0; i < NumberOfBuckshots; i++)
    {
        const FVector RandVector = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
        const FVector EndLocation = SphereCenter + RandVector;
        FVector ToEndLocation = EndLocation - TraceStart;
        ToEndLocation = TraceStart + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size();

        HitTargets.Add(ToEndLocation);
    }

}
