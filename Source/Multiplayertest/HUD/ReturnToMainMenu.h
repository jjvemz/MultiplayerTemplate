// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReturnToMainMenu.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTEST_API UReturnToMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	void MenuSetup();

	void MenuTearDown();


protected:
virtual bool Initialize() override;

void OnPlayerLeftGame();

UFUNCTION()
void OnDestroySession(bool bWasSuccesful);

private:
    UPROPERTY(meta =(BindWidget))
    class UButton* ReturnMMenuButton;

    UFUNCTION()
    void ReturnButtonClicked();

    UPROPERTY()

    class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;
    UPROPERTY()

    class APlayerController* PlayerController;
};
