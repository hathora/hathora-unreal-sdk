// Copyright 2023 Hathora, Inc.


#include "SDKDemoGameModeBase.h"

#include "DemoHUD.h"

ASDKDemoGameModeBase::ASDKDemoGameModeBase()
{
	PlayerControllerClass = APlayerController::StaticClass();
	HUDClass = ADemoHUD::StaticClass();
}