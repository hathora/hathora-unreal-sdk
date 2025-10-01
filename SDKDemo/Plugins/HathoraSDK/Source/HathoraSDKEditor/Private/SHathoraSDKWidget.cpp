// Copyright 2023 Hathora, Inc.

#include "SHathoraSDKWidget.h"
#include "HathoraSDK.h"
#include "HathoraSDKAppV1.h"
#include "SlateBasics.h"
#include "Styling/AppStyle.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Layout/SScaleBox.h"

#define LOCTEXT_NAMESPACE "SHathoraSDKWidget"

void SHathoraSDKWidget::Construct(const FArguments& InArgs)
{
	TSharedRef<SVerticalBox> LoggedInWidget = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("DeveloperTokenLabel", "Developer Token"))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SAssignNew(DeveloperTokenTextBox, SEditableTextBox)
					.IsPassword(true)
				]
			]
			// + SVerticalBox::Slot()
			// .AutoHeight()
			// [
			// 	SNew(SButton)
			// 	.Text(LOCTEXT("LoggedInButtonText", "[Logged In] Log in with another account"))
			// ]
		];

	TSharedRef<SVerticalBox> Applications = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ApplicationsLabel", "Target Application"))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(ApplicationComboBox, SApplicationComboBox)
				.OptionsSource(&ApplicationsList)
				.OnSelectionChanged(this, &SHathoraSDKWidget::ApplicationSelected)
				.OnGenerateWidget(this, &SHathoraSDKWidget::GenerateApplicationComboBoxItem)
				[
					SNew(SBox)
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.OnClicked(this, &SHathoraSDKWidget::RefreshApplications)
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("GenericCommands.Redo"))
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ApplicationIdLabel", "AppId"))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(AppIdTextBlock, STextBlock)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("DataTableEditor.Copy.Small"))
				]
			]
		];

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			LoggedInWidget
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			Applications
		]
	];
}

TSharedRef<SWidget> SHathoraSDKWidget::GenerateApplicationComboBoxItem(FHathoraApplicationPtr Item) const
{
	return SNew(STextBlock)
		.Text(FText::FromString(Item->AppName))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 16));
}

FReply SHathoraSDKWidget::RefreshApplications()
{
	UHathoraSDK* SDK = UHathoraSDK::CreateHathoraSDK();
	SDK->SetAuthToken(DeveloperTokenTextBox->GetText().ToString());
	if (SDK->IsLoggedIn())
	{
		SDK->AppV1->GetApps(
			UHathoraSDKAppV1::FHathoraOnGetApps::CreateLambda(
				[this](const FHathoraGetAppsResult Result)
				{
					ApplicationsList.Empty();

					if (!Result.ErrorMessage.IsEmpty())
					{
						return;
					}

					for (FHathoraApplication Application : Result.Data)
					{
						ApplicationsList.Add(MakeShareable(new FHathoraApplication(Application)));
					}

					ApplicationComboBox->RefreshOptions();
				}
			)
		);
	}

	return FReply::Handled();
}

void SHathoraSDKWidget::ApplicationSelected(FHathoraApplicationPtr ProposedSelection, ESelectInfo::Type SelectInfo)
{
	if (ProposedSelection.IsValid())
	{
		AppIdTextBlock->SetText(FText::FromString(*ProposedSelection->AppId));
	}
}

#undef LOCTEXT_NAMESPACE
