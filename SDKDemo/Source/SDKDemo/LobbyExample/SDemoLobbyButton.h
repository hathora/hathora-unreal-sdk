// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SDemoLobbyButton : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SDemoLobbyButton)
	{
		_ButtonText = TEXT("");
		_OnClicked = FOnClicked();
	}
	SLATE_ARGUMENT(FString, ButtonText)
	SLATE_EVENT(FOnClicked, OnClicked)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
