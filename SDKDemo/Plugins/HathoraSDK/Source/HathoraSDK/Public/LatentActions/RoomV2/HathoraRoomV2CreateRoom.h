// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HathoraTypes.h"
#include "HathoraRoomV2LatentCommon.h"
#include "HathoraRoomV2CreateRoom.generated.h"

UCLASS()
class HATHORASDK_API UHathoraRoomV2CreateRoom : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

	// Create a new room for an existing application. Poll the GetConnectionInfo()
	// endpoint to get connection details for an active room.
	// @param Region The region to create the room in.
	// @param RoomConfig Optional configuration parameters for the room. Can be
	//                   any string including stringified JSON. It is accessible
	//                   from the room via GetRoomInfo().
	// @param RoomId Unique identifier to a game session or match. Leave empty to
	//               use the default system generated ID.
	UFUNCTION(
		BlueprintCallable,
		meta =
			(BlueprintInternalUseOnly = "true",
			 Category = "HathoraSDK | RoomV2",
			 WorldContext = "WorldContextObject")
	)
	static UHathoraRoomV2CreateRoom *CreateRoom(
		UHathoraSDKRoomV2 *HathoraSDKRoomV2,
		UObject *WorldContextObject,
		EHathoraCloudRegion Region,
		FString RoomConfig,
		FString RoomId
	);

	UPROPERTY(BlueprintAssignable)
	FHathoraRoomV2OnRoomConnectionInfo OnComplete;

	UPROPERTY()
	UHathoraSDKRoomV2 *HathoraSDKRoomV2;

	EHathoraCloudRegion Region;
	FString RoomConfig;
	FString RoomId;
};