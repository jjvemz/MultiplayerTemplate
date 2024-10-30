// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTEST_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta=(BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(meta=(BindWidget))
	 UTextBlock* PointsText;

	UPROPERTY(meta=(BindWidget))
	 UTextBlock* DefeatsAmount;

	 UPROPERTY(meta = (BindWidget))
	 UTextBlock* WeaponAmmoAmount;

	 UPROPERTY(meta = (BindWidget))
	 UTextBlock* MagCapacity;

     UPROPERTY(meta = (BindWidget))
     UTextBlock* GrenadesText;

	 UPROPERTY(meta = (BindWidget))
	 UTextBlock* MatchCountdownText;

     UPROPERTY(meta = (BindWidget))
     UProgressBar* ShieldBar;

     UPROPERTY(meta = (BindWidget))
     class UImage* HighPingImage;

     UPROPERTY(meta = (BindWidgetAnim), Transient)
     UWidgetAnimation* HighPingAnim;
  
};
