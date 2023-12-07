// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HathoraTypes.h"
#include "HathoraRoomV2LatentCommon.h"
#include "HathoraRoomV2DestroyRoom.generated.h"

UCLASS()
class HATHORASDK_API UHathoraRoomV2DestroyRoom : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

	// Destroy a room. All associated metadata is deleted.
	// @param RoomId Unique identifier to a game session or match.
	UFUNCTION(
		BlueprintCallable,
		meta =
			(BlueprintInternalUseOnly = "true",
			 Category = "HathoraSDK | RoomV2",
			 WorldContext = "WorldContextObject")
	)
	static UHathoraRoomV2DestroyRoom *DestroyRoom(
		UHathoraSDKRoomV2 *HathoraSDKRoomV2,
		UObject *WorldContextObject,
		FString RoomId
	);

	UPROPERTY(BlueprintAssignable)
	FHathoraRoomV2OnDestroyRoom OnComplete;

	UPROPERTY()
	UHathoraSDKRoomV2 *HathoraSDKRoomV2;

	FString RoomId;
};