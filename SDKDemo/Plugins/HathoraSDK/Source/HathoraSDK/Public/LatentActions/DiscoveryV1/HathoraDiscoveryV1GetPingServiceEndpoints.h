// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HathoraTypes.h"
#include "HathoraSDKDiscoveryV1.h"
#include "HathoraDiscoveryV1GetPingServiceEndpoints.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FHathoraDiscoveryV1GetPingServiceEndpointsComplete, const TArray<FHathoraDiscoveredPingEndpoint>&, Endpoints
);

UCLASS()
class HATHORASDK_API UHathoraDiscoveryV1GetPingServiceEndpoints : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

	// Returns an array of all regions with a host that a client can ping via ICMP.
	// See the "Get Regional Pings" functions that will handle the full ping process for you.
	// @param OnComplete The delegate to call when the request is complete.
	UFUNCTION(
		BlueprintCallable,
		meta =
			(BlueprintInternalUseOnly = "true",
			 Category = "HathoraSDK | DiscoveryV1",
			 WorldContext = "WorldContextObject")
	)
	static UHathoraDiscoveryV1GetPingServiceEndpoints *GetPingServiceEndpoints(
		UHathoraSDKDiscoveryV1 *HathoraSDKDiscoveryV1,
		UObject *WorldContextObject
	);

	UPROPERTY(BlueprintAssignable)
	FHathoraDiscoveryV1GetPingServiceEndpointsComplete OnComplete;

	UPROPERTY()
	UHathoraSDKDiscoveryV1 *HathoraSDKDiscoveryV1;
};