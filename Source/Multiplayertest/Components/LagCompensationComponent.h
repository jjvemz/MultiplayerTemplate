// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"


USTRUCT(BlueprintType)
struct FBoxInformation
{
    GENERATED_BODY()

    UPROPERTY()
    FVector Location;

    UPROPERTY()
    FRotator Rotation;

    UPROPERTY()
    FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
    GENERATED_BODY()

    UPROPERTY()
    float Time;

    UPROPERTY()
    TMap<FName, FBoxInformation> HitBoxInfo;

    //No es necesario para las Hitscan pero si para las escopetas
    UPROPERTY()
    AShooterPlayer* ShooterPlayer;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
    GENERATED_BODY()

    UPROPERTY()
    bool bHitConfirmed;

    UPROPERTY()
    bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
    GENERATED_BODY()

    UPROPERTY()
    TMap<AShooterPlayer*, uint32> HeadShots;

    UPROPERTY()
    TMap<AShooterPlayer*, uint32> BodyShots;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERTEST_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
    friend class AShooterPlayer;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void ShowFramePackage(const FFramePackage& Package, const FColor& Color);

    //Para las armas HitScans

    FServerSideRewindResult ServerSideRewind(class AShooterPlayer* HitCharacter,
        const FVector_NetQuantize& TraceStart, 
        const FVector_NetQuantize& HitLocation, float HitTime);


    //Para las armas de projectiles

    FServerSideRewindResult ProjectileServerSideRewind(
        AShooterPlayer* HitCharacter,
        const FVector_NetQuantize& TraceStart,
        const FVector_NetQuantize100& InitialVelocity,
        float HitTime
    );


    //Para las Escopetas
    FShotgunServerSideRewindResult ShotgunServerSideRewind(
        const TArray<AShooterPlayer*>& HitCharacters,
        const FVector_NetQuantize& TraceStart,
        const TArray<FVector_NetQuantize>& HitLocations,
        float HitTime);


    UFUNCTION(Server, Reliable)
    void ServerScoreRequest(
        AShooterPlayer* HitCharacter,
        const FVector_NetQuantize& TraceStart,
        const FVector_NetQuantize& HitLocation,
        float HitTime,
        class AWeaponActor* DamageCauser
    );


    UFUNCTION(Server, Reliable)
   void ProjectileServerScoreRequest(
    AShooterPlayer* HitCharacter,
    const FVector_NetQuantize& TraceStart,
    const FVector_NetQuantize100& InitialVelocity,
    float HitTime
   );

    UFUNCTION(Server, Reliable)
    void ShotgunServerScoreRequest(
        const TArray<AShooterPlayer*>& HitCharacters,
        const FVector_NetQuantize& TraceStart,
        const TArray<FVector_NetQuantize>& HitLocations,
        float HitTime
    );

protected:
	virtual void BeginPlay() override;
    void SaveFramePackage(FFramePackage& Package);
    FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);

    void CacheBoxPositions(AShooterPlayer* HitCharacter, FFramePackage& OutFramePackage);
    void MoveBoxes(AShooterPlayer* HitCharacter, const FFramePackage& Package);
    void ResetHitBoxes(AShooterPlayer* HitCharacter, const FFramePackage& Package);
    void EnableCharacterMeshCollision(AShooterPlayer* HitCharacter, ECollisionEnabled::Type CollisionEnabled);

    void SaveFramePackage();
    FFramePackage GetFrameToCheck(AShooterPlayer* HitCharacter, float HitTime);

    //HitScan
        FServerSideRewindResult ConfirmHit(
        const FFramePackage& Package,
        AShooterPlayer* HitCharacter,
        const FVector_NetQuantize& TraceStart,
        const FVector_NetQuantize& HitLocation);

    //Para lsos proyectiles

    FServerSideRewindResult ProjectileConfirmHit(
        const FFramePackage& Package,
        AShooterPlayer* HitCharacter,
        const FVector_NetQuantize& TraceStart,
        const FVector_NetQuantize100& InitialVelocity,
        float HitTime
    );

    //Shotgun
 

    FShotgunServerSideRewindResult ShotgunConfirmHit(
        const TArray<FFramePackage>& FramePackages,
        const FVector_NetQuantize& TraceStart,
        const TArray<FVector_NetQuantize>& HitLocations
    );
private:	

    UPROPERTY()
    AShooterPlayer* ShooterCharacter;

    UPROPERTY()
    class AShooterPlayerController* ShooterController;
		

    TDoubleLinkedList<FFramePackage> FrameHistory;

    UPROPERTY(EditAnywhere)
    float MaxRecordTime = 4.f; 
};
