// Copyright 2023 Hathora, Inc.

#include "DemoMatchWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/GridPanel.h"
#include "Components/GridSlot.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "DemoMatchGameState.h"
#include "../DemoRoomConfigFunctionLibrary.h"

void UDemoMatchWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

bool UDemoMatchWidget::Initialize()
{
	bool Success = Super::Initialize();

	if (!Success) return false;

	// Set up UI
	{
		UCanvasPanel* RootPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
		check(RootPanel);

		WidgetTree->RootWidget = RootPanel;

		UGridPanel* GridPanel = WidgetTree->ConstructWidget<UGridPanel>(UGridPanel::StaticClass());
		check(GridPanel);

		UCanvasPanelSlot* GridPanelSlot = RootPanel->AddChildToCanvas(GridPanel);
		GridPanelSlot->SetPosition(FVector2D(50, 50));
		GridPanelSlot->SetAutoSize(true);

		UTextBlock* NameLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		check(NameLabel);
		NameLabel->SetText(FText::FromString(TEXT("Room Name:")));
		GridPanel->AddChildToGrid(NameLabel, 0, 0);

		UTextBlock* CodeLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		check(CodeLabel);
		CodeLabel->SetText(FText::FromString(TEXT("Room Code:")));
		GridPanel->AddChildToGrid(CodeLabel, 1, 0);

		UTextBlock* TimeLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		check(TimeLabel);
		TimeLabel->SetText(FText::FromString(TEXT("Match Time:")));
		GridPanel->AddChildToGrid(TimeLabel, 2, 0);

		RoomName = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		check(RoomName);
		GridPanel->AddChildToGrid(RoomName, 0, 1);

		RoomCode = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		check(RoomCode);
		GridPanel->AddChildToGrid(RoomCode, 1, 1);

		MatchTime = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		check(MatchTime);
		GridPanel->AddChildToGrid(MatchTime, 2, 1);
	}

	GameState = Cast<ADemoMatchGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (GameState)
	{
		GameState->OnMatchTimeUpdated.AddDynamic(this, &UDemoMatchWidget::ProcessMatchTime);

		// Allow some buffer time for replication
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			[this]()
			{
				if (GameState->bLobbyIsReady)
				{
					ProcessRoomConfig();
				}
				else
				{
					GameState->OnLobbyReady.AddDynamic(this, &UDemoMatchWidget::ProcessRoomConfig);
				}
			},
			0.5f,
			false
		);
	}

	return true;
}

void UDemoMatchWidget::ProcessRoomConfig()
{
	RoomCode->SetText(FText::FromString(GameState->LobbyInfo.ShortCode));

	FString RoomNameString = UDemoRoomConfigFunctionLibrary::DeserializeRoomConfigFromString(GameState->LobbyInfo.RoomConfig).RoomName;
	RoomName->SetText(FText::FromString(RoomNameString));
}

void UDemoMatchWidget::ProcessMatchTime(int32 InMatchTime)
{
	FString MatchTimeString = InMatchTime < 0 ?
		FString::Printf(TEXT("Starts in %ds"), InMatchTime) :
		FString::Printf(TEXT("%ds elapsed"), InMatchTime);

	MatchTime->SetText(FText::FromString(MatchTimeString));
}
