// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HathoraTypes.h"
#include "HathoraRoomV2LatentCommon.h"
#include "HathoraRoomV2SuspendRoom.generated.h"

UCLASS()
class HATHORASDK_API UHathoraRoomV2SuspendRoom : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

	// Suspend a room. The room is unallocated from the process but
	// can be rescheduled later using the same roomId.
	// @param RoomId Unique identifier to a game session or match.
	UFUNCTION(
		BlueprintCallable,
		meta =
			(BlueprintInternalUseOnly = "true",
			 Category = "HathoraSDK | RoomV2",
			 WorldContext = "WorldContextObject")
	)
	static UHathoraRoomV2SuspendRoom *SuspendRoom(
		UHathoraSDKRoomV2 *HathoraSDKRoomV2,
		UObject *WorldContextObject,
		FString RoomId
	);

	UPROPERTY(BlueprintAssignable)
	FHathoraRoomV2OnSuspendRoom OnComplete;

	UPROPERTY()
	UHathoraSDKRoomV2 *HathoraSDKRoomV2;

	FString RoomId;
};