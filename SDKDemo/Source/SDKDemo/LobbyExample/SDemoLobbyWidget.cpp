// Copyright 2023 Hathora, Inc.

#include "SDemoLobbyWidget.h"
#include "SlateBasics.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Layout/SScaleBox.h"

void SDemoLobbyWidget::Construct(const FArguments& InArgs)
{
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
				SAssignNew(ErrorMessageBlock, STextBlock)
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
							.OnClicked(InArgs._OnRefreshClicked)
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
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						.Padding(FMargin(10.0f, 0.0f, 10.0f, 10.0f))
						[
							SAssignNew(LobbiesGrid, SGridPanel)
							.FillColumn(0, 1.5f)
							.FillColumn(1, 2.0f)
							.FillColumn(2, 1.0f)
							.FillColumn(3, 1.0f)
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
						[
							SAssignNew(LoginSwitcher, SWidgetSwitcher)
							.WidgetIndex(0)
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
									.OnClicked(InArgs._OnLoginClicked)
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
										.OnClicked(InArgs._OnSwitchToPublicLobbyClicked)
										[
											SAssignNew(PublicButtonTextBlock, STextBlock)
											.Text(FText::FromString("Public"))
											.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
										]
									]
									+ SHorizontalBox::Slot()
									.HAlign(HAlign_Fill)
									.VAlign(VAlign_Fill)
									[
										SNew(SButton)
										.OnClicked(InArgs._OnSwitchToPrivateLobbyClicked)
										[
											SAssignNew(PrivateButtonTextBlock, STextBlock)
											.Text(FText::FromString("Private"))
											.Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
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
										SAssignNew(RegionComboBox, SRegionComboBox)
										.OptionsSource(InArgs._RegionList)
										.OnSelectionChanged(InArgs._OnRegionSelected)
										.OnGenerateWidget(this, &SDemoLobbyWidget::HandleGenerateWidget)
										[
											SNew(SBox)
										]
									]
									+ SHorizontalBox::Slot()
									.HAlign(HAlign_Fill)
									.VAlign(VAlign_Fill)
									.AutoWidth()
									[
										SNew(SBox)
										.WidthOverride(100.0f)
										[
											SAssignNew(RegionPingTextBlock, STextBlock)
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
									SAssignNew(RoomNameTextBox, SEditableTextBox)
									.HintText(FText::FromString("Room Name"))
									.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
									.Justification(ETextJustify::Center)
								]
								+ SVerticalBox::Slot()
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Fill)
								.Padding(FMargin(0.0f, 10.0f, 0.0f, 20.0f))
								[
									SAssignNew(CreateButton, SButton)
									.OnClicked(InArgs._OnCreateLobbyClicked)
									[
										SAssignNew(CreateButtonTextBlock, STextBlock)
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
								SAssignNew(RoomCodeTextBox, SEditableTextBox)
								.HintText(FText::FromString("Room Code"))
								.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
								.Justification(ETextJustify::Center)
							]
							+ SVerticalBox::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Fill)
							.Padding(FMargin(0.0f, 10.0f, 0.0f, 20.0f))
							[
								SAssignNew(JoinButton, SButton)
								.OnClicked(InArgs._OnJoinClicked)
								[
									SAssignNew(JoinButtonTextBlock, STextBlock)
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

void SDemoLobbyWidget::SetErrorMessage(const FString& Message)
{
	ErrorMessageBlock->SetText(FText::FromString(Message));
}

void SDemoLobbyWidget::SetLoggedIn()
{
	LoginSwitcher->SetActiveWidgetIndex(1);
}

void SDemoLobbyWidget::SetLobbyVisibility(bool bPublic)
{
	PublicButtonTextBlock->SetFont(FCoreStyle::GetDefaultFontStyle(bPublic ? "Bold" : "Regular", 16));
	PrivateButtonTextBlock->SetFont(FCoreStyle::GetDefaultFontStyle(bPublic ? "Regular" : "Bold", 16));
}

void SDemoLobbyWidget::SetCreatingLobby(bool bCreating)
{
	CreateButton->SetEnabled(!bCreating);
	CreateButtonTextBlock->SetText(FText::FromString(bCreating ? "Creating..." : "Create"));
	JoinButton->SetEnabled(!bCreating);
}

void SDemoLobbyWidget::SetJoiningLobby(bool bJoining)
{
	CreateButton->SetEnabled(!bJoining);
	JoinButton->SetEnabled(!bJoining);
	JoinButtonTextBlock->SetText(FText::FromString(bJoining ? "Joining..." : "Join"));
}

void SDemoLobbyWidget::SetRegionPing(int32 Ping)
{
	RegionPingTextBlock->SetText(FText::FromString(FString::Printf(TEXT("%dms"), Ping)));
}

TSharedRef<SWidget> SDemoLobbyWidget::HandleGenerateWidget(TSharedPtr<FString> Item) const
{
	return SNew(STextBlock)
		.Text(FText::FromString(*Item))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 16));
}