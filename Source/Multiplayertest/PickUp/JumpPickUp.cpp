// Fill out your copyright notice in the Description page of Project Settings.


#include "JumpPickUp.h"
#include "Multiplayertest/Character/ShooterPlayer.h"
#include "Multiplayertest/Components/BuffComponent.h"


void AJumpPickUp::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    AShooterPlayer* ShooterPlayer = Cast<AShooterPlayer>(OtherActor);
    
    if (ShooterPlayer)
    {
        UBuffComponent* BuffComp = ShooterPlayer->GetBuff();
        if (BuffComp)
        {
            BuffComp->BuffJump(JumpZVelocityBuff, JumpBuffTime);
        }
    }

    Destroy();
}
