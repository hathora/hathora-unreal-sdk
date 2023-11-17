// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HathoraTypes.h"
#include "HathoraLobbyV3LatentCommon.h"
#include "HathoraLobbyV3CreateLobby.generated.h"

UCLASS()
class HATHORASDK_API UHathoraLobbyV3CreateLobby : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

	// Create a new lobby for an application. A lobby object is a wrapper around a room object.
	// With a lobby, you get additional functionality like configuring the visibility of the room,
	// managing the state of a match, and retrieving a list of public lobbies to display to players.
	// @param Visibility Types of lobbies a player can create.
	// @param RoomConfig Optional configuration parameters for the room. Can be
	//                   any string including stringified JSON. It is accessible
	//                   from the room via GetRoomInfo().
	// @param Region The region to create the room in.
	// @param ShortCode Optional user-defined identifier for a lobby. Leave empty
	//                  to reference the lobby using the RoomId only.
	// @param RoomId Unique identifier to a game session or match. Leave empty to
	//               use the default system generated ID.
	UFUNCTION(
		BlueprintCallable,
		meta =
			(BlueprintInternalUseOnly = "true",
			 Category = "HathoraSDK | LobbyV3",
			 WorldContext = "WorldContextObject")
	)
	static UHathoraLobbyV3CreateLobby *CreateLobby(
		UHathoraSDKLobbyV3 *HathoraSDKLobbyV3,
		UObject *WorldContextObject,
		EHathoraLobbyVisibility Visibility,
		FString RoomConfig,
		EHathoraCloudRegion Region,
		FString ShortCode,
		FString RoomId
	);

	UPROPERTY(BlueprintAssignable)
	FHathoraLobbyV3OnLobbyInfo OnComplete;

	UPROPERTY()
	UHathoraSDKLobbyV3 *HathoraSDKLobbyV3;

	EHathoraLobbyVisibility Visibility;
	FString RoomConfig;
	EHathoraCloudRegion Region;
	FString ShortCode;
	FString RoomId;
};