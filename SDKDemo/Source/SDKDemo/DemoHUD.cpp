// Copyright 2023 Hathora, Inc.

#include "DemoHUD.h"
#include "DemoMenuWidget.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"

void ADemoHUD::BeginPlay()
{
	Super::BeginPlay();

	if (GEngine && GEngine->GameViewport)
	{
		DemoMenuWidget = CreateWidget<UDemoMenuWidget>(GetWorld(), UDemoMenuWidget::StaticClass());
		check(DemoMenuWidget);
		DemoMenuWidget->AddToViewport();
	}

	GetOwningPlayerController()->SetInputMode(FInputModeUIOnly());
}
