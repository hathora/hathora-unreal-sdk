// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HathoraTypes.h"
#include "HathoraRoomV2LatentCommon.h"
#include "HathoraRoomV2GetActiveRoomsForProcess.generated.h"

UCLASS()
class HATHORASDK_API UHathoraRoomV2GetActiveRoomsForProcess : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

	// Get all active rooms for a given process.
	// @param ProcessId System generated unique identifier to a runtime instance
	//                  of your game server.
	UFUNCTION(
		BlueprintCallable,
		meta =
			(BlueprintInternalUseOnly = "true",
			 Category = "HathoraSDK | RoomV2",
			 WorldContext = "WorldContextObject")
	)
	static UHathoraRoomV2GetActiveRoomsForProcess *GetActiveRoomsForProcess(
		UHathoraSDKRoomV2 *HathoraSDKRoomV2,
		UObject *WorldContextObject,
		FString ProcessId
	);

	UPROPERTY(BlueprintAssignable)
	FHathoraRoomV2OnGetRoomsForProcess OnComplete;

	UPROPERTY()
	UHathoraSDKRoomV2 *HathoraSDKRoomV2;

	FString ProcessId;
};