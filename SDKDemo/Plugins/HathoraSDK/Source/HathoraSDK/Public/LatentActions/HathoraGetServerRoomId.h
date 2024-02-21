// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HathoraTypes.h"
#include "HathoraGetServerRoomId.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FHathoraGetServerRoomIdComplete, FString, RoomId
);

UCLASS()
class HATHORASDK_API UHathoraGetServerRoomId : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

	// Get the current Room ID for this Unreal process. Call this from a server.
	// @param PollingInterval The number of seconds to wait between polling to retrieve the Room ID.
	UFUNCTION(
		BlueprintCallable,
		meta =
			(BlueprintInternalUseOnly = "true",
			 Category = "HathoraSDK",
			 WorldContext = "WorldContextObject")
	)
	static UHathoraGetServerRoomId *GetServerRoomId(
		UObject *WorldContextObject,
		float PollingInterval = 1.0f
	);

	UPROPERTY(BlueprintAssignable)
	FHathoraGetServerRoomIdComplete OnComplete;

	float PollingInterval;
};