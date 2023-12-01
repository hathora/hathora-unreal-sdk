// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SDemoLobbyWidget.h"
#include "DemoLobbyWidget.generated.h"

class UHathoraLobbyComponent;

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
	UPROPERTY()
	UHathoraLobbyComponent* HathoraLobby;

	TSharedPtr<SDemoLobbyWidget> LobbyWidget;

	TArray<TSharedPtr<FString>> RegionList;
	bool bCreatePublicLobby = true;

	UFUNCTION()
	void OnError(FString InErrorMessage);

	UFUNCTION()
	void OnLobbyReady(
		FHathoraConnectionInfo ConnectionInfo,
		FString RoomConfigString,
		EHathoraCloudRegion Region
	);

	int32 NumPublicLobbies = 0;
	TMap<FString, int32> Pings;
	void StartPings();
	void UpdateSelectedRegionPing();

	void RefreshPublicLobbies();
	void Login();
	void CreateLobby();
	void JoinLobby();
};
