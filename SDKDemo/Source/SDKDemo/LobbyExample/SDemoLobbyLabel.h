// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SDemoLobbyLabel : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SDemoLobbyLabel)
	{
		_LabelText = TEXT("");
	}
	SLATE_ARGUMENT(FString, LabelText)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
