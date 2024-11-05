// Fill out your copyright notice in the Description page of Project Settings.


#include "ReturnToMainMenu.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameModebase.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"


void UReturnToMainMenu:: MenuSetup()
{

    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    bIsFocusable = true;

    UWorld * World= GetWorld();
    if(World)
    {
        PlayerController = PlayerController == nullptr ?  World->GetFirstPlayerController(): PlayerController;
        if(PlayerController)
        {
            FInputModeGameAndUI InputModeData;
            InputModeData.SetWidgetToFocus(TakeWidget());
            PlayerController->SetInputMode(InputModeData);
            PlayerController->SetShowMouseCursor(true);
        }
    }

    if (ReturnMMenuButton && !ReturnMMenuButton->OnClicked.IsBound())
    {
        ReturnMMenuButton->OnClicked.AddDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);
    }
    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance)
    {
        MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
        if (MultiplayerSessionsSubsystem && !MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsBound())
        {
            MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UReturnToMainMenu::OnDestroySession);
        }
    }
}


void UReturnToMainMenu::ReturnButtonClicked()
{
    ReturnMMenuButton->SetIsEnabled(false);
    if (MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->DestroySession();
    }
}


void UReturnToMainMenu::OnDestroySession(bool bWasSuccesful)
{
    if (!bWasSuccesful)
    {
        ReturnMMenuButton->SetIsEnabled(true);
        return;
    }
    UWorld* World = GetWorld();
    if (World)
    {
        AGameModeBase* GameMode= World->GetAuthGameMode<AGameModeBase>();
        if (GameMode) //Caso en que el servidor se desconecte
        {
            GameMode->ReturnToMainMenuHost();
        }
        else 
        {
            PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
            if (PlayerController) //Caso en que un cliente se desconencte
            {
                PlayerController->ClientReturnToMainMenuWithTextReason(FText());
            }
        }
    }

}


void UReturnToMainMenu::MenuTearDown()
{
    RemoveFromParent();
    UWorld * World= GetWorld();
    if(World)
    {
        PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
        if(PlayerController)
        {
            FInputModeGameOnly InputModeData;
            PlayerController->SetInputMode(InputModeData);
            PlayerController->SetShowMouseCursor(false);
        }
    }
    if (ReturnMMenuButton && ReturnMMenuButton->OnClicked.IsBound())
    {
        ReturnMMenuButton->OnClicked.RemoveDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);
    }
    if (MultiplayerSessionsSubsystem && MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsBound())
    {
        MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UReturnToMainMenu::OnDestroySession);
    }
}

bool UReturnToMainMenu::Initialize()
{
      if(!Super::Initialize())
      {
        return false;
      }

    return true;
}

