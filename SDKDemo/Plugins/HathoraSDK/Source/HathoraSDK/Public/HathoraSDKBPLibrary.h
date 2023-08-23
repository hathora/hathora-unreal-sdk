// Copyright 2023 Hathora, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "HathoraSDK.h"
#include "HathoraSDKTypes.h"

#include "HathoraSDKBPLibrary.generated.h"

UCLASS()
class UFHathoraSDKBPLibrary : public UBlueprintFunctionLibrary {
	GENERATED_BODY()


	UDELEGATE()
	DECLARE_DYNAMIC_DELEGATE_OneParam(FPingServiceEndpointsDynamicDelegate, const TArray<FDiscoveredPingEndpoint>&, PingEndpoints);
	/**
	 * Returns an array of all regions with a host and port that a client can directly ping.
	 * Use the GetRegionalPings method to get the actual ping values.
	 */
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK")
	static void GetPingServiceEndpoints(const FPingServiceEndpointsDynamicDelegate& OnComplete);


	UDELEGATE()
	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGetRegionalPingsDynamicDelegate, FRegionalPings, Result);
	/**
	 * Calls GetPingServiceEndpoints to get all available regions, opens a WebSocket connection
	 * with each, and returns the latency in milliseconds for each endpoint in a map keyed by region name.
	 */
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK")
	static void GetAllRegionalPings(const FOnGetRegionalPingsDynamicDelegate& OnComplete);


	UDELEGATE()
	DECLARE_DYNAMIC_DELEGATE_ThreeParams(FConnectionInfoDynamicDelegate, bool, Success, FString, Host, int32, Port);
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
