// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HathoraTypes.h"
#include "HathoraRoomV2LatentCommon.h"
#include "HathoraRoomV2UpdateRoomConfig.generated.h"

UCLASS()
class HATHORASDK_API UHathoraRoomV2UpdateRoomConfig : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

	// Update the roomConfig variable for a room.
	// @param RoomId Unique identifier to a game session or match.
	// @param RoomConfig Optional configuration parameters for the room. Can be
	//                   any string including stringified JSON. It is accessible
	//                   from the room via UpdateRoomConfig().
	UFUNCTION(
		BlueprintCallable,
		meta =
			(BlueprintInternalUseOnly = "true",
			 Category = "HathoraSDK | RoomV2",
			 WorldContext = "WorldContextObject")
	)
	static UHathoraRoomV2UpdateRoomConfig *UpdateRoomConfig(
		UHathoraSDKRoomV2 *HathoraSDKRoomV2,
		UObject *WorldContextObject,
		FString RoomId,
		FString RoomConfig
	);

	UPROPERTY(BlueprintAssignable)
	FHathoraRoomV2OnUpdateRoomConfig OnComplete;

	UPROPERTY()
	UHathoraSDKRoomV2 *HathoraSDKRoomV2;

	FString RoomId;
	FString RoomConfig;
};