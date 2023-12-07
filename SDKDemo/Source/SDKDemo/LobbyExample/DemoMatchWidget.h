// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DemoMatchWidget.generated.h"

class ADemoMatchGameState;
class UTextBlock;

UCLASS()
class UDemoMatchWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void ProcessRoomConfig();

protected:
	virtual void NativeConstruct() override;
	virtual bool Initialize() override;

private:
	ADemoMatchGameState* GameState;

	UTextBlock* RoomName;
	UTextBlock* RoomCode;
	UTextBlock* MatchTime;

	UFUNCTION()
	void ProcessMatchTime(int32 InMatchTime);
};
