// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HathoraTypes.h"
#include "HathoraLobbyV3LatentCommon.h"
#include "HathoraLobbyV3ListAllActivePublicLobbies.generated.h"

UCLASS()
class HATHORASDK_API UHathoraLobbyV3ListAllActivePublicLobbies : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

	// Get all public active lobbies for a given application. Use this endpoint to display
	// all public lobbies that a player can join in the game client. Use
	// ListRegionActivePublicLobbies() to only see lobbies in a specific region.
	UFUNCTION(
		BlueprintCallable,
		meta =
			(BlueprintInternalUseOnly = "true",
			 Category = "HathoraSDK | LobbyV3",
			 WorldContext = "WorldContextObject")
	)
	static UHathoraLobbyV3ListAllActivePublicLobbies *ListAllActivePublicLobbies(
		UHathoraSDKLobbyV3 *HathoraSDKLobbyV3,
		UObject *WorldContextObject
	);

	UPROPERTY(BlueprintAssignable)
	FHathoraLobbyV3OnLobbyInfos OnComplete;

	UPROPERTY()
	UHathoraSDKLobbyV3 *HathoraSDKLobbyV3;
};