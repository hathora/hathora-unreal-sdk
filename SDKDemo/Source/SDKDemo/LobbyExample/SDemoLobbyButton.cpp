// Copyright 2023 Hathora, Inc.

#include "SDemoLobbyButton.h"
#include "SlateBasics.h"

void SDemoLobbyButton::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBorder)
		[
			SNew(SBox)
			.Padding(FMargin(10.0f, 10.0f, 10.0f, 10.0f))
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.OnClicked(InArgs._OnClicked)
				[
					SNew(STextBlock)
					.Text(FText::FromString(InArgs._ButtonText))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
					.Justification(ETextJustify::Center)
				]
			]
		]
	];
}
