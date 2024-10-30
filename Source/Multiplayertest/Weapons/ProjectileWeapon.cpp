// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
    UWorld* World = GetWorld();

	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();
        
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = GetOwner();
        SpawnParams.Instigator = InstigatorPawn;

        AProjectile* SpawnedProjectile = nullptr;
		if (ProjectileClass && InstigatorPawn)
		{
            if (bUseServerSideRewind) 
            {
                if (InstigatorPawn->HasAuthority()) //Servidor
                {

                    if (InstigatorPawn->IsLocallyControlled()) //Servidor, host - usar proyectiles replicados
                    {
                        SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
                        SpawnedProjectile->bUseServerSideRewind = false;
                        SpawnedProjectile->Damage = Damage;
                    }
                    else //Servidor, No es controlado localmente- Spawnear projectiles no replicados, Sin SSR
                    {
                        SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
                        SpawnedProjectile->bUseServerSideRewind = false;
                    }
                }
                else //Cliente, con SSR
                {
                    if(InstigatorPawn->IsLocallyControlled()) // Cliente, controlado localmente - Spawnear projectiles no replicados, con SSR
                    {
                        SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
                        SpawnedProjectile->bUseServerSideRewind = true;
                        SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
                        SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
                        SpawnedProjectile->Damage = Damage;
                    }
                    else  // Cliente, no controlado localmente - Spawnear projectiles no replicados, sin SSR
                    {
                        SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
                        SpawnedProjectile->bUseServerSideRewind = false;
                    }
                }
            }
            else //arma sin usar SSR
            {
                if (InstigatorPawn->HasAuthority())
                {
                    SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
                    SpawnedProjectile->bUseServerSideRewind = false;
                    SpawnedProjectile->Damage = Damage;
                }
            }
		}
	}
}
