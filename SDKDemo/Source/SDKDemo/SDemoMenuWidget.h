// Copyright 2023 Hathora, Inc.

#pragma once

#include "SlateBasics.h"

/**
 *
 */
class SDemoMenuWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDemoMenuWidget) {}
	SLATE_ARGUMENT(TWeakObjectPtr<class ADemoHUD>, OwningHUD)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	FReply OnClicked() const;

	TWeakObjectPtr<class AMenuHUD> OwningHUD;

	virtual bool SupportsKeyboardFocus() const override { return true; }
};
