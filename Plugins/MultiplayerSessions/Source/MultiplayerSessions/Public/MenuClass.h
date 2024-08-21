// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuClass.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenuClass : public UUserWidget
{
	GENERATED_BODY()
	UFUNCTION(BlueprintCallable)
	void MenuSetup();
};
