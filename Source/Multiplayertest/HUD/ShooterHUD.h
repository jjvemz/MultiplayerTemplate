// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ShooterHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:
	class UTexture2D* CrosshairsCenter;
	class UTexture2D* CrosshairsLeft;
	class UTexture2D* CrosshairsRight;
	class UTexture2D* CrosshairsUp;
	class UTexture2D* CrosshairsDown;
	float CrosshairSpread;
	FLinearColor CrosshairColor;

};

/**
 * 
 */
UCLASS()
class MULTIPLAYERTEST_API AShooterHUD : public AHUD
{
	GENERATED_BODY()
	
	virtual void DrawHUD() override;
private:
	FHUDPackage HUDPackage;

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrossHairColor);
public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
