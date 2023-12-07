// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HathoraTypes.h"
#include "HathoraRoomV2LatentCommon.h"
#include "HathoraRoomV2GetConnectionInfo.generated.h"

UCLASS()
class HATHORASDK_API UHathoraRoomV2GetConnectionInfo : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

	// Poll this endpoint to get connection details to a room.
	// Clients can call this endpoint without authentication.
	// @param RoomId Unique identifier to a game session or match.
	UFUNCTION(
		BlueprintCallable,
		meta =
			(BlueprintInternalUseOnly = "true",
			 Category = "HathoraSDK | RoomV2",
			 WorldContext = "WorldContextObject")
	)
	static UHathoraRoomV2GetConnectionInfo *GetConnectionInfo(
		UHathoraSDKRoomV2 *HathoraSDKRoomV2,
		UObject *WorldContextObject,
		FString RoomId
	);

	UPROPERTY(BlueprintAssignable)
	FHathoraRoomV2OnRoomConnectionInfo OnComplete;

	UPROPERTY()
	UHathoraSDKRoomV2 *HathoraSDKRoomV2;

	FString RoomId;
};