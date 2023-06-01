// Copyright 2023 Hathora, Inc.

#include "DemoHUD.h"
#include "SDemoMenuWidget.h"
#include "Engine/Engine.h"
void ADemoHUD::BeginPlay()
{
	Super::BeginPlay();

	if (GEngine && GEngine->GameViewport)
	{
		DemoMenuWidget = SNew(SDemoMenuWidget).OwningHUD(this);
		GEngine->GameViewport->AddViewportWidgetContent(SAssignNew(
			DemoMenuWidgetContainer, SWeakWidget ).PossiblyNullContent(DemoMenuWidget.ToSharedRef()));
	}
}
