// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickUp.h"

#include "multiplayertest/Character/ShooterPlayer.h"
#include "multiplayertest/Components/CombatComponent.h"

void AAmmoPickUp::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
    bool bFromSweep, const FHitResult& SweepResult)
{
    Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);


    AShooterPlayer* ShooterPlayer = Cast<AShooterPlayer>(OtherActor);
    if (ShooterPlayer)
    {
        UCombatComponent* Combatcmpt = ShooterPlayer->GetCombat();
        if (Combatcmpt)
        {
            Combatcmpt->PickUpAmmo(WeaponType, AmmoAmount);
        }
    }
    Destroy();
}
