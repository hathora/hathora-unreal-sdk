// Copyright 2023 Hathora, Inc.

#include "DemoLobbyHUD.h"
#include "HathoraLobbyComponent.h"
#include "DemoLobbyWidget.h"

ADemoLobbyHUD::ADemoLobbyHUD(
  const FObjectInitializer &ObjectInitializer
) : Super(ObjectInitializer)
{
	HathoraLobby = CreateDefaultSubobject<UHathoraLobbyComponent>(TEXT("HathoraLobby"));
}

void ADemoLobbyHUD::BeginPlay()
{
	Super::BeginPlay();

	UDemoLobbyWidget* DemoLobbyWidget = CreateWidget<UDemoLobbyWidget>(GetWorld(), UDemoLobbyWidget::StaticClass());
	if (DemoLobbyWidget)
	{
		DemoLobbyWidget->AddToViewport();

		APlayerController* PlayerController = GetOwningPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(DemoLobbyWidget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputMode);
			PlayerController->bShowMouseCursor = true;
		}
	}
}