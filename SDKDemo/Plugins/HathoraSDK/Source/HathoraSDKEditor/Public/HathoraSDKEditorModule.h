// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class SDockTab;
class FSpawnTabArgs;

DECLARE_LOG_CATEGORY_EXTERN(LogHathoraSDKEditorModule, Log, All)

class FHathoraSDKEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TSharedPtr<class FUICommandList> PluginCommands;

	static FName TabNameOpenTab;

	void ButtonClicked();
	TSharedRef<SDockTab> CreatePluginTab(const FSpawnTabArgs& SpawnTabArgs);
};
