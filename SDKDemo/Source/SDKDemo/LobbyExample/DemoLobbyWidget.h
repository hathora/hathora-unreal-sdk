// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DemoLobbyWidget.generated.h"

UCLASS()
class UDemoLobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:

protected:
	virtual void NativeConstruct() override;
	virtual bool Initialize() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
};
