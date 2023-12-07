// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "HathoraSDKAppV1.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
// #include "Widgets/Layout/SGridPanel.h"

class SHathoraSDKWidget : public SCompoundWidget
{
public:
	typedef TSharedPtr<FHathoraApplication> FHathoraApplicationPtr;
	typedef SComboBox<FHathoraApplicationPtr> SApplicationComboBox;

	SLATE_BEGIN_ARGS(SHathoraSDKWidget)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedRef<SWidget> GenerateApplicationComboBoxItem(FHathoraApplicationPtr Item) const;
	void ApplicationSelected(FHathoraApplicationPtr ProposedSelection, ESelectInfo::Type SelectInfo);

	FReply RefreshApplications();

	TSharedPtr<SEditableTextBox> DeveloperTokenTextBox;
	TSharedPtr<SApplicationComboBox> ApplicationComboBox;
	TArray<FHathoraApplicationPtr> ApplicationsList;
	TSharedPtr<STextBlock> AppIdTextBlock;
};
