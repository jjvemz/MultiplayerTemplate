// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUp.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

#include "Multiplayertest/Weapons/WeaponTypes.h"


APickUp::APickUp()
{

	PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
    OverlapSphere->SetupAttachment(RootComponent);
    OverlapSphere->SetSphereRadius(150.f);
    OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    OverlapSphere->AddLocalOffset(FVector(0.f, 0.f, 85.f));
   
    PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
    PickupMesh->SetupAttachment(OverlapSphere);
    PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PickupMesh->SetRelativeScale3D(FVector(5.f, 5.f, 5.f));
    PickupMesh->SetRenderCustomDepth(true);
    PickupMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);

    PickupEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffectComponent"));
    PickupEffectComponent->SetupAttachment(RootComponent);

}

void APickUp::BeginPlay()
{
	Super::BeginPlay();
	
    if (HasAuthority())
    {
        //OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &APickUp::OnSphereOverlap);
        
        GetWorldTimerManager().SetTimer(
            BindOverlapTimer,
            this, &APickUp::BindOverlapTimerFinished,
            BindOverlapTime
        );
    }
}

void APickUp::BindOverlapTimerFinished()
{
    OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &APickUp::OnSphereOverlap);
}


void APickUp::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}

void APickUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (PickupMesh)
    {
        PickupMesh->AddWorldRotation(FRotator(0.f, TurnRate * DeltaTime, 0.f));
    }

}

void APickUp::Destroyed()
{
    Super::Destroyed();

    if (PickupSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            PickupSound,
            GetActorLocation()
        );
    }

    if (PickupEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            this,
            PickupEffect,
            GetActorLocation(),
            GetActorRotation()
        );
    }
}



