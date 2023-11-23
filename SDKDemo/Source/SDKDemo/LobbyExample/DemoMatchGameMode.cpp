// Copyright 2023 Hathora, Inc.

#include "DemoMatchGameMode.h"
#include "DemoMatchGameState.h"
#include "DemoMatchHUD.h"

ADemoMatchGameMode::ADemoMatchGameMode(
  const FObjectInitializer &ObjectInitializer
) :
  Super(ObjectInitializer) {
  GameStateClass = ADemoMatchGameState::StaticClass();
  HUDClass = ADemoMatchHUD::StaticClass();
}
