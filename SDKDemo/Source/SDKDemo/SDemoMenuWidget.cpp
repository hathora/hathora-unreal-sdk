// Copyright 2023 Hathora, Inc.


#include "SDemoMenuWidget.h"

#include "HathoraPing.h"

#define LOCTEXT_NAMESPACE "Menu"
void SDemoMenuWidget::Construct(const FArguments& InArgs)
{
	const FMargin ContentPadding = FMargin(500.f, 300.f);
	const FText ButtonText = LOCTEXT("Button Text", "Get Pings");
	ChildSlot
	[
	SNew(SOverlay) + SOverlay::Slot()
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SImage)
		.ColorAndOpacity(FColor::Black)
    ]
    + SOverlay::Slot()
    .HAlign(HAlign_Fill)
    .VAlign(VAlign_Fill)
    .Padding(ContentPadding)
    [
    	SNew(SVerticalBox)
    + SVerticalBox::Slot()
    [
    	SNew(SButton)
    	.OnClicked(this, &SDemoMenuWidget::OnClicked)
    	[
    		SNew(STextBlock)
    		.Text(ButtonText)
    	]
    ]
    ]]; 
}

FReply SDemoMenuWidget::OnClicked() const
{
	const UHathoraPing::FOnGetRegionalPingsDelegate OnComplete = UHathoraPing::FOnGetRegionalPingsDelegate::CreateLambda([](
		TMap<FString, int32> Results) {
		for (const auto& Pair : Results)
		{
			const FString& Region = Pair.Key;
			UE_LOG(LogTemp, Display, TEXT("Ping in %s is %d ms"), *Region, Pair.Value);	
		}
	});
	UHathoraPing::GetRegionalPings(OnComplete);
	return FReply::Handled();
}
#undef LOCTEXT_NAMESPACE
