// Copyright Epic Games, Inc. All Rights Reserved.

#include "HathoraSDKEditorModule.h"
#include "HathoraSDKEditorCommands.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "ToolMenus.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "SHathoraSDKWidget.h"

#define LOCTEXT_NAMESPACE "FHathoraSDKEditorModule"

DEFINE_LOG_CATEGORY(LogHathoraSDKEditorModule)
IMPLEMENT_MODULE(FHathoraSDKEditorModule, HathoraSDKEditorModule)

FName FHathoraSDKEditorModule::TabNameOpenTab(TEXT("HathoraSDKOpenTab"));

void FHathoraSDKEditorModule::StartupModule()
{
	FHathoraSDKEditorCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FHathoraSDKEditorCommands::Get().OpenWindow,
		FExecuteAction::CreateRaw(this, &FHathoraSDKEditorModule::ButtonClicked),
		FCanExecuteAction()
	);

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TabNameOpenTab,FOnSpawnTab::CreateRaw(this, &FHathoraSDKEditorModule::CreatePluginTab))
		.SetDisplayName(LOCTEXT("FHathoraSDKTabTitle", "Hathora SDK"))
		.SetTooltipText(LOCTEXT("FHathoraSDKTooltipText", "Open the Hathora SDK development tools."))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory());

}

void FHathoraSDKEditorModule::ShutdownModule()
{
	if (FSlateApplication::IsInitialized())
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabNameOpenTab);
	}
}

void FHathoraSDKEditorModule::ButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(TabNameOpenTab);
}

TSharedRef<SDockTab> FHathoraSDKEditorModule::CreatePluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SHathoraSDKWidget)
		];
}

#undef LOCTEXT_NAMESPACE