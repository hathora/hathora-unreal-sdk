// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DemoLobbyHUD.generated.h"

class UHathoraLobbyComponent;

UCLASS()
class ADemoLobbyHUD : public AHUD
{
	GENERATED_UCLASS_BODY()

public:
	UHathoraLobbyComponent* GetHathoraLobby() const { return HathoraLobby; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UHathoraLobbyComponent* HathoraLobby;
};
