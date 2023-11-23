// Copyright 2023 Hathora, Inc.

#include "DemoLobbyHUD.h"
// #include "DemoLobbyWidget.h"

void ADemoLobbyHUD::BeginPlay()
{
	Super::BeginPlay();

	// UDemoLobbyWidget* DemoLobbyWidget = CreateWidget<UDemoLobbyWidget>(GetWorld(), UDemoLobbyWidget::StaticClass());
	// if (DemoLobbyWidget)
	// {
	// 	DemoLobbyWidget->AddToViewport();

	// 	APlayerController* PlayerController = GetOwningPlayerController();
	// 	if (PlayerController)
	// 	{
	// 		FInputModeUIOnly InputMode;
	// 		InputMode.SetWidgetToFocus(DemoLobbyWidget->TakeWidget());
	// 		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	// 		PlayerController->SetInputMode(InputMode);
	// 		PlayerController->bShowMouseCursor = true;
	// 	}
	// }
}