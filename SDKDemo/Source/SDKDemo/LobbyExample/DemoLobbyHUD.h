// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DemoLobbyHUD.generated.h"

UCLASS()
class ADemoLobbyHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
};
