// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketMovementComponent.h"
#include "RocketMovementComponent.h"

URocketMovementComponent::EHandleBlockingHitResult URocketMovementComponent::HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
        Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);
        return EHandleBlockingHitResult::AdvanceNextSubstep;
}

void URocketMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
    //El cohete no debería parar, solo explotar cuando su caja de colisión detecta un impacto
}
