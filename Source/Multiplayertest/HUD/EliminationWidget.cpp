// Fill out your copyright notice in the Description page of Project Settings.


#include "EliminationWidget.h"
#include "Components/TextBlock.h"

void UEliminationWidget::SetEliminationAnnouncementText(FString AttackerName, FString VictimName)
{
    FString EliminationAnnouncementText = FString::Printf(TEXT("%s descartó a %s!!"), *AttackerName, *VictimName);
    if (AnnouncementText)
    {
        AnnouncementText->SetText(FText::FromString(EliminationAnnouncementText));
    }
}
