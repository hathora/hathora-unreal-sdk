// Copyright 2023 Hathora, Inc.

#include "DemoMatchHUD.h"
#include "DemoMatchWidget.h"

void ADemoMatchHUD::BeginPlay()
{
	Super::BeginPlay();

	UDemoMatchWidget* DemoMatchWidget = CreateWidget<UDemoMatchWidget>(GetWorld(), UDemoMatchWidget::StaticClass());
	if (DemoMatchWidget)
	{
		DemoMatchWidget->AddToViewport();

		APlayerController* PlayerController = GetOwningPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputMode;
			PlayerController->SetInputMode(InputMode);
			PlayerController->bShowMouseCursor = false;
		}
	}
}