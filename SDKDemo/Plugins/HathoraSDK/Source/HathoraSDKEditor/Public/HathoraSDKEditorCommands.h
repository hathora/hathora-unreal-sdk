// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

#define LOCTEXT_NAMESPACE "FHathoraSDKEditorCommands"

class FHathoraSDKEditorCommands : public TCommands< FHathoraSDKEditorCommands > {
public:
	FHathoraSDKEditorCommands() :
		TCommands< FHathoraSDKEditorCommands >(
			TEXT("HathoraSDK"),
			FText::FromString("Hathora SDK"),
			NAME_None,
			"HathoraSDKEditorCommandsStyle"
		) {}

	TSharedPtr< FUICommandInfo > OpenWindow;

	virtual void RegisterCommands() override {
		UI_COMMAND(
			OpenWindow,
			"HathoraSDK",
			"Open the Hathora SDK window",
			EUserInterfaceActionType::Button,
			FInputChord());
	}
};

#undef LOCTEXT_NAMESPACE
