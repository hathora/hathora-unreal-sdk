// Copyright 2023 Hathora, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "HathoraSDK.h"
#include "RegionalPings.h"

#include "HathoraSDKBPLibrary.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(FPingServiceEndpointsDynamicDelegate, const TArray<FDiscoveredPingEndpoint>&, PingEndpoints);

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGetRegionalPingsDynamicDelegate, FRegionalPings, Result);

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FConnectionInfoDynamicDelegate, bool, Success, FString, Host, int32, Port);

UCLASS()
class UFHathoraSDKBPLibrary : public UBlueprintFunctionLibrary {
	GENERATED_BODY()

	/**
	 * Returns an array of all regions with a host and port that a client can directly ping.
	 * Use the GetRegionalPings method to get the actual ping values.
	 */
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK")
	static void GetPingServiceEndpoints(const FPingServiceEndpointsDynamicDelegate& OnComplete);

	/**
	 * Opens a WebSocket connection with each of the provided PingEndpoints and returns
	 * the latency in milliseconds for each endpoint in a map keyed by region name.
	 *
	 * @param PingEndpoints Array of PingEndpoints to ping; if empty, GetPingServiceEndpoints will be
	 * called to get the list of endpoints to ping.
	 */
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK")
	static void GetRegionalPings(TArray<FDiscoveredPingEndpoint> PingEndpoints, const FOnGetRegionalPingsDynamicDelegate& OnComplete);

	/**
	 * Get the hostname + port given an AppId and RoomId
	 * This allows matchmaking services to return a result while the room is starting
	 * and not yet active.
	 *
	 * @param AppId System generated unique identifier for an application. (e.g. app-af469a92-5b45-4565-b3c4-b79878de67d2)
	 * @param RoomId Unique identifier to a game session or match. Use either a system generated ID or pass in your own.
	 */
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK")
	static void GetConnectionInfo(const FString& AppId, const FString& RoomId, const FConnectionInfoDynamicDelegate& OnComplete);
};
