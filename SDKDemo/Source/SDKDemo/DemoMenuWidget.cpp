// Copyright 2023 Hathora, Inc.

#include "DemoMenuWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "HathoraSDK.h"

#define LOCTEXT_NAMESPACE "Menu"

void UDemoMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

bool UDemoMenuWidget::Initialize()
{
	bool response = Super::Initialize();
	if (!response)
	{
		return false;
	}

	const FMargin ContentPadding = FMargin(500.f, 300.f);
	const FText ButtonText = LOCTEXT("Button Text", "Get Pings");

	UCanvasPanel* RootPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
	check(RootPanel);

	WidgetTree->RootWidget = RootPanel;

	USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	check(SizeBox);

	SizeBox->SetWidthOverride(500);
	SizeBox->SetHeightOverride(500);

	UCanvasPanelSlot* SizeBoxSlot = RootPanel->AddChildToCanvas(SizeBox);

	// Center the size box in the canvas panel
	FAnchorData SizeBoxAnchorData;
	SizeBoxAnchorData.Anchors = FAnchors(0.5f);
	SizeBoxAnchorData.Alignment = FVector2D(0.5f, 0.5f);
	SizeBoxSlot->SetLayout(SizeBoxAnchorData);
	SizeBoxSlot->SetPosition(FVector2D::ZeroVector);
	SizeBoxSlot->SetAutoSize(true);

	UBorder* BorderWidget = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	check(BorderWidget);

	BorderWidget->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
	BorderWidget->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
	BorderWidget->SetBrushColor(FLinearColor(0, 0, 0, 0.2));

	UVerticalBox* ContainerBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	check(ContainerBox);

	RegionList = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	check(RegionList);

	UButton* PingRegionsButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	check(PingRegionsButton);

	UTextBlock* ButtonLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	check(ButtonLabel);

	ButtonLabel->SetText(ButtonText);
	PingRegionsButton->AddChild(ButtonLabel);

	UVerticalBoxSlot* RegionListSlot = ContainerBox->AddChildToVerticalBox(RegionList);
	RegionListSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	ContainerBox->AddChildToVerticalBox(PingRegionsButton);

	PingRegionsButton->OnClicked.AddDynamic(this, &UDemoMenuWidget::InitiatePing);

	BorderWidget->SetContent(ContainerBox);
	SizeBox->SetContent(BorderWidget);

	return true;
}

void UDemoMenuWidget::InitiatePing()
{
	RegionList->ClearChildren();
	UHathoraSDK::GetPingsForRegions(
		UHathoraSDK::GetRegionMap(),
		EHathoraPingType::ICMP,
		FHathoraOnGetRegionalPings::CreateLambda(
			[this](const FHathoraRegionPings& Result)
			{
				for (const auto& Pair : Result.Pings)
				{
					FString Region = Pair.Key;
					int32 RTT = Pair.Value;
					UE_LOG(LogTemp, Display, TEXT("Ping in %s is %d ms"), *Region, RTT);

					UHorizontalBox* RegionBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
					check(RegionBox);

					UTextBlock* RegionLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
					check(RegionLabel);
					RegionLabel->SetText(FText::FromString(Region));

					UTextBlock* RegionPing = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
					check(RegionPing);
					RegionPing->SetText(FText::FromString(FString::Printf(TEXT("%dms"), RTT)));

					UHorizontalBoxSlot* LabelSlot = RegionBox->AddChildToHorizontalBox(RegionLabel);
					LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
					RegionBox->AddChildToHorizontalBox(RegionPing);

					RegionList->AddChildToVerticalBox(RegionBox);
				}
			}
		)
	);
}

#undef LOCTEXT_NAMESPACE
