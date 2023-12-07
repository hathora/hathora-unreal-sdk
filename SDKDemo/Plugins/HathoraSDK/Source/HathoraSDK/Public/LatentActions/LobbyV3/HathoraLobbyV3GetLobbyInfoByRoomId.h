// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HathoraTypes.h"
#include "HathoraLobbyV3LatentCommon.h"
#include "HathoraLobbyV3GetLobbyInfoByRoomId.generated.h"

UCLASS()
class HATHORASDK_API UHathoraLobbyV3GetLobbyInfoByRoomId : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

	// Get details for a lobby.
	// @param RoomId Unique identifier to a game session or match.
	UFUNCTION(
		BlueprintCallable,
		meta =
			(BlueprintInternalUseOnly = "true",
			 Category = "HathoraSDK | LobbyV3",
			 WorldContext = "WorldContextObject")
	)
	static UHathoraLobbyV3GetLobbyInfoByRoomId *GetLobbyInfoByRoomId(
		UHathoraSDKLobbyV3 *HathoraSDKLobbyV3,
		UObject *WorldContextObject,
		FString RoomId
	);

	UPROPERTY(BlueprintAssignable)
	FHathoraLobbyV3OnLobbyInfo OnComplete;

	UPROPERTY()
	UHathoraSDKLobbyV3 *HathoraSDKLobbyV3;

	FString RoomId;
};