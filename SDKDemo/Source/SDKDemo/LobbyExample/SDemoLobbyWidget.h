// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SDemoLobbyWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDemoLobbyWidget)
	{
		_ErrorMessage = TEXT("");
		_bIsLoggedIn = false;
		_bCreatePublicLobby = true;
		_RegionList = TArray<TSharedPtr<FString>>();
	}
	SLATE_ARGUMENT(FString, ErrorMessage)
	SLATE_ARGUMENT(bool, bIsLoggedIn)
	SLATE_ARGUMENT(bool, bCreatePublicLobby)
	SLATE_ARGUMENT(TArray<TSharedPtr<FString>>, RegionList)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	TSharedPtr<SBox> ComboBox;
};
