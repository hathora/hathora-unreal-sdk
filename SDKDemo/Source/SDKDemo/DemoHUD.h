// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DemoHUD.generated.h"

/**
 *
 */
UCLASS()
class SDKDEMO_API ADemoHUD : public AHUD
{
	GENERATED_BODY()
protected:
	class UDemoMenuWidget* DemoMenuWidget;

	virtual void BeginPlay() override;
};
