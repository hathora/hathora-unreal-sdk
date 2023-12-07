// Copyright 2023 Hathora, Inc.

#include "DemoLobbyGameMode.h"
#include "DemoLobbyHUD.h"

ADemoLobbyGameMode::ADemoLobbyGameMode(
  const FObjectInitializer &ObjectInitializer
) :
  Super(ObjectInitializer) {
  HUDClass = ADemoLobbyHUD::StaticClass();
}
