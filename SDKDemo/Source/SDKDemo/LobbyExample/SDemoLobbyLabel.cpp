// Copyright 2023 Hathora, Inc.

#include "SDemoLobbyLabel.h"
#include "SlateBasics.h"
#include "Widgets/Layout/SScaleBox.h"

void SDemoLobbyLabel::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBorder)
		[
			SNew(SBox)
			.Padding(FMargin(10.0f, 10.0f, 10.0f, 10.0f))
			[
				SNew(SScaleBox)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.Stretch(EStretch::ScaleToFit)
				.StretchDirection(EStretchDirection::DownOnly)
				[
					SNew(STextBlock)
					.Text(FText::FromString(InArgs._LabelText))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
					.Justification(ETextJustify::Center)
				]
			]
		]
	];
}
