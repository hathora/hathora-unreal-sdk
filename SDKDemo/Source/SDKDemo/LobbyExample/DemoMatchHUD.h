// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DemoMatchHUD.generated.h"

UCLASS()
class ADemoMatchHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
};
