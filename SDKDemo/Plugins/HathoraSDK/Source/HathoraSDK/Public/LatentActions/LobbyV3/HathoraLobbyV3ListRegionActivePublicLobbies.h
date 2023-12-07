// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HathoraTypes.h"
#include "HathoraLobbyV3LatentCommon.h"
#include "HathoraLobbyV3ListRegionActivePublicLobbies.generated.h"

UCLASS()
class HATHORASDK_API UHathoraLobbyV3ListRegionActivePublicLobbies : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

	// Get all active lobbies for a given application, filtered by Region.
	// Use this endpoint to display all public lobbies that a player can join
	// in the game client. Use ListAllActivePublicLobbies() to see all lobbies.
	// @param Region Filter the returned lobbies by the provided region.
	UFUNCTION(
		BlueprintCallable,
		meta =
			(BlueprintInternalUseOnly = "true",
			 Category = "HathoraSDK | LobbyV3",
			 WorldContext = "WorldContextObject")
	)
	static UHathoraLobbyV3ListRegionActivePublicLobbies *ListRegionActivePublicLobbies(
		UHathoraSDKLobbyV3 *HathoraSDKLobbyV3,
		UObject *WorldContextObject,
		EHathoraCloudRegion Region
	);

	UPROPERTY(BlueprintAssignable)
	FHathoraLobbyV3OnLobbyInfos OnComplete;

	UPROPERTY()
	UHathoraSDKLobbyV3 *HathoraSDKLobbyV3;

	EHathoraCloudRegion Region;
};