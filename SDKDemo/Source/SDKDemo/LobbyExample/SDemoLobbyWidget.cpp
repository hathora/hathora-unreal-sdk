// Copyright 2023 Hathora, Inc.

#include "SDemoLobbyWidget.h"
#include "SlateBasics.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Layout/SScaleBox.h"

void SDemoLobbyWidget::Construct(const FArguments& InArgs)
{
	FSlateBrush BorderBrush;
	BorderBrush.TintColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.0f);
	BorderBrush.DrawAs = ESlateBrushDrawType::Type::RoundedBox;

	FSlateBrush ToggleButtonBrushLeft;
	ToggleButtonBrushLeft.DrawAs = ESlateBrushDrawType::Type::RoundedBox;
	ToggleButtonBrushLeft.OutlineSettings = FSlateBrushOutlineSettings(FVector4(10.0f, 0.0f, 0.0f, 10.0f));
	FButtonStyle LobbyVisibilityButtonLeftStyle;
	LobbyVisibilityButtonLeftStyle.SetNormal(ToggleButtonBrushLeft);

	FSlateBrush ToggleButtonBrushRight;
	ToggleButtonBrushRight.DrawAs = ESlateBrushDrawType::Type::RoundedBox;
	ToggleButtonBrushRight.OutlineSettings = FSlateBrushOutlineSettings(FVector4(0.0f, 10.0f, 10.0f, 0.0f));
	FButtonStyle LobbyVisibilityButtonRightStyle;
	LobbyVisibilityButtonRightStyle.SetNormal(ToggleButtonBrushRight);

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SImage)
			.ColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.2f))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.Padding(FMargin(200.0f, 0.0f, 200.0f, 0.0f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(InArgs._ErrorMessage))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
				.Justification(ETextJustify::Center)
				.ColorAndOpacity(FLinearColor(0.692708f, 0.054118f, 0.054118f, 1.0f))
			]
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Top)
				.FillWidth(1.0f)
				.Padding(FMargin(15.0f, 15.0f, 15.0f, 15.0f))
				[
					SNew(SBorder)
					// .BorderImage(&BorderBrush)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						.Padding(FMargin(0.0f, 20.0f, 0.0f, 20.0f))
						[
							SNew(STextBlock)
							.Text(FText::FromString("JOIN PUBLIC GAME"))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
							.Justification(ETextJustify::Center)
						]
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Right)
						.VAlign(VAlign_Fill)
						.Padding(FMargin(0.0f, 0.0f, 10.0f, 0.0f))
						[
							SNew(SButton)
							[
								SNew(STextBlock)
								.Text(FText::FromString("Refresh"))
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
							]
						]
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						.Padding(FMargin(10.0f, 10.0f, 10.0f, 0.0f))
						[
							SNew(SGridPanel)
							.FillColumn(0, 1.5f)
							.FillColumn(1, 2.0f)
							.FillColumn(2, 1.0f)
							.FillColumn(3, 1.0f)
							+ SGridPanel::Slot(0, 0)
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							[
								SNew(STextBlock)
								.Text(FText::FromString("Room ID"))
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
								.Justification(ETextJustify::Center)
							]
							+ SGridPanel::Slot(1, 0)
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							[
								SNew(STextBlock)
								.Text(FText::FromString("Name"))
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
								.Justification(ETextJustify::Center)
							]
							+ SGridPanel::Slot(2, 0)
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							[
								SNew(STextBlock)
								.Text(FText::FromString("Ping"))
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
								.Justification(ETextJustify::Center)
							]
							+ SGridPanel::Slot(3, 0)
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
						]
					]
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.FillWidth(0.5f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.AutoHeight()
					.Padding(FMargin(15.0f, 15.0f, 15.0f, 15.0f))
					[
						SNew(SBorder)
						// .BorderImage(&BorderBrush)
						[
							SNew(SWidgetSwitcher)
							.WidgetIndex(1)
							+ SWidgetSwitcher::Slot()
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								.Padding(FMargin(20.0f, 20.0f, 20.0f, 20.0f))
								[
									SNew(SScaleBox)
									.Stretch(EStretch::ScaleToFit)
									.StretchDirection(EStretchDirection::DownOnly)
									[
										SNew(STextBlock)
										.Text(FText::FromString("LOGIN TO CREATE GAME"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
										.Justification(ETextJustify::Center)
									]
								]
								+ SVerticalBox::Slot()
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								.Padding(50.0f, 0.0f, 50.0f, 0.0f)
								[
									SNew(STextBlock)
									.Text(FText::FromString("You need to login before you can create a game."))
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
								]
								+ SVerticalBox::Slot()
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Fill)
								.Padding(0.0f, 10.0f, 0.0f, 20.0f)
								[
									SNew(SButton)
									[
										SNew(STextBlock)
										.Text(FText::FromString("Login"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
									]
								]
							]
							+ SWidgetSwitcher::Slot()
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								.Padding(FMargin(20.0f, 20.0f, 20.0f, 20.0f))
								[
									SNew(SScaleBox)
									.Stretch(EStretch::ScaleToFit)
									.StretchDirection(EStretchDirection::DownOnly)
									[
										SNew(STextBlock)
										.Text(FText::FromString("CREATE GAME"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
										.Justification(ETextJustify::Center)
									]
								]
								+ SVerticalBox::Slot()
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Fill)
								.Padding(FMargin(0.0f, 0.0f, 0.0f, 10.0f))
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
									.HAlign(HAlign_Fill)
									.VAlign(VAlign_Fill)
									[
										SNew(SButton)
										// .ButtonStyle(&LobbyVisibilityButtonLeftStyle)
										[
											SNew(STextBlock)
											.Text(FText::FromString("Public"))
											.Font(FCoreStyle::GetDefaultFontStyle(InArgs._bCreatePublicLobby ? "Bold" : "Regular", 16))
										]
									]
									+ SHorizontalBox::Slot()
									.HAlign(HAlign_Fill)
									.VAlign(VAlign_Fill)
									[
										SNew(SButton)
										// .ButtonStyle(&LobbyVisibilityButtonRightStyle)
										[
											SNew(STextBlock)
											.Text(FText::FromString("Private"))
											.Font(FCoreStyle::GetDefaultFontStyle(InArgs._bCreatePublicLobby ? "Regular" : "Bold", 16))
										]
									]
								]
								+ SVerticalBox::Slot()
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								.Padding(FMargin(15.0f, 0.0f, 15.0f, 10.0f))
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
									.HAlign(HAlign_Fill)
									.VAlign(VAlign_Fill)
									.FillWidth(1.0f)
									[
										SNew(SComboBox<TSharedPtr<FString>>)
										.OptionsSource(&InArgs._RegionList)
									]
									+ SHorizontalBox::Slot()
									.HAlign(HAlign_Fill)
									.VAlign(VAlign_Fill)
									.AutoWidth()
									[
										SNew(SBox)
										.WidthOverride(100.0f)
										[
											SNew(STextBlock)
											.Text(FText::FromString(FString::Printf(TEXT("%dms"), 0))) // todo region ping
											.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
											.Justification(ETextJustify::Right)
										]
									]
								]
								+ SVerticalBox::Slot()
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								.Padding(FMargin(15.0f, 0.0f, 15.0f, 10.0f))
								[
									SNew(SEditableTextBox)
									.HintText(FText::FromString("Room Name"))
									.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
									.ForegroundColor(FLinearColor::Black)
									.FocusedForegroundColor(FLinearColor::Black)
									.Justification(ETextJustify::Center)
								]
								+ SVerticalBox::Slot()
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Fill)
								.Padding(FMargin(0.0f, 10.0f, 0.0f, 20.0f))
								[
									SNew(SButton)
									[
										SNew(STextBlock)
										.Text(FText::FromString("Create"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
									]
								]
							]
						]
					]
					+ SVerticalBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.AutoHeight()
					.Padding(FMargin(15.0f, 15.0f, 15.0f, 15.0f))
					[
						SNew(SBorder)
						// .BorderImage(&BorderBrush)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							.Padding(FMargin(20.0f, 20.0f, 20.0f, 20.0f))
							[
								SNew(SScaleBox)
								.Stretch(EStretch::ScaleToFit)
								.StretchDirection(EStretchDirection::DownOnly)
								[
									SNew(STextBlock)
									.Text(FText::FromString("JOIN GAME"))
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
									.Justification(ETextJustify::Center)
								]
							]
							+ SVerticalBox::Slot()
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							.Padding(FMargin(15.0f, 0.0f, 15.0f, 10.0f))
							[
								SNew(SEditableTextBox)
								.HintText(FText::FromString("Room Code"))
								.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
								.ForegroundColor(FLinearColor::Black)
								.FocusedForegroundColor(FLinearColor::Black)
								.Justification(ETextJustify::Center)
							]
							+ SVerticalBox::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Fill)
							.Padding(FMargin(0.0f, 10.0f, 0.0f, 20.0f))
							[
								SNew(SButton)
								[
									SNew(STextBlock)
									.Text(FText::FromString("Join"))
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
								]
							]
						]
					]
				]
			]
		]
	];
}