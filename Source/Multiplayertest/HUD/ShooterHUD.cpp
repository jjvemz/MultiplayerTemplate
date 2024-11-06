// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterHUD.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "GameFramework/PlayerController.h"

#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"

#include "Announcement.h"
#include "CharacterOverlay.h"
#include "EliminationWidget.h"


void AShooterHUD::BeginPlay()
{
	Super::BeginPlay();
	//AddCharacterOverlay();
}

void AShooterHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void AShooterHUD::DrawHUD()
{
	Super::DrawHUD(); 

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;
		if (HUDPackage.CrosshairsCenter)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsRight)
		{
			FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsUp)
		{
			FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsUp, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsDown)
		{
			FVector2D Spread(0.f, SpreadScaled);

			DrawCrosshair(HUDPackage.CrosshairsDown, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
	}
}

void AShooterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrossHairColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y
	);

	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		CrossHairColor
	);
}

void AShooterHUD::AddAnnouncement()
{

	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && AnnouncementClass)
	{
		Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);
		Announcement->AddToViewport();
	}
}

void AShooterHUD::AddEliminationAnnouncement(FString Attacker, FString Victim)
{
    OwningPlayer = OwningPlayer == nullptr ? GetOwningPlayerController() : OwningPlayer;
    if (OwningPlayer && EliminationAnnouncementClass)
    {
        UEliminationWidget* EliminationAnouncementWidget = CreateWidget<UEliminationWidget>(OwningPlayer, EliminationAnnouncementClass);
        if (EliminationAnouncementWidget)
        {
            EliminationAnouncementWidget->SetEliminationAnnouncementText(Attacker, Victim);
            EliminationAnouncementWidget->AddToViewport();

            for (UEliminationWidget* Msg : EliminationMessages)
            {
                if (Msg && Msg->AnnouncementBox)
                {
                    UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->AnnouncementBox);
                    if (CanvasSlot) 
                    {
                        FVector2D Pos = CanvasSlot->GetPosition();
                        FVector2D NewPosition(
                            CanvasSlot->GetPosition().X,
                            Pos.Y - CanvasSlot->GetSize().Y
                        );
                        CanvasSlot->SetPosition(NewPosition);
                    }
                }
            }

            EliminationMessages.Add(EliminationAnouncementWidget);

            FTimerHandle EliminationMsgTimer;
            FTimerDelegate EliminationMsgDelegate;
            EliminationMsgDelegate.BindUFunction(this, FName("EliminationAnnouncementTimerFinished"), EliminationAnouncementWidget);
            GetWorldTimerManager().SetTimer(
                EliminationMsgTimer,
                EliminationMsgDelegate,
                EliminationAnnouncementTime,
                false
            );
        }
    }
}


void AShooterHUD::EliminationAnnouncementTimerFinished(UEliminationWidget* MsgToRemove)
{
    if (MsgToRemove)
    {
        MsgToRemove->RemoveFromParent();
    }
}
