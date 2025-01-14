// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HathoraTypes.h"
#include "HathoraSDKDiscoveryV2.h"
#include "HathoraDiscoveryV2GetPingServiceEndpoints.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FHathoraDiscoveryV2GetPingServiceEndpointsComplete, const TArray<FHathoraDiscoveredPingEndpoint>&, Endpoints
);

UCLASS()
class HATHORASDK_API UHathoraDiscoveryV2GetPingServiceEndpoints : public UBlueprintAsyncActionBase
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
			 Category = "HathoraSDK | DiscoveryV2",
			 WorldContext = "WorldContextObject",
			 DeprecatedFunction,
			 DeprecationMessage = "Manually specify the regions you'd like to ping in UHathoraSDK::GetPingsForRegions (recommended) or call UHathoraSDK::GetRegionMap to get the URLs for all regions (for this version of the plugin).")
	)
	static UHathoraDiscoveryV2GetPingServiceEndpoints *GetPingServiceEndpoints(
		UHathoraSDKDiscoveryV2 *HathoraSDKDiscoveryV2,
		UObject *WorldContextObject
	);

	UPROPERTY(BlueprintAssignable)
	FHathoraDiscoveryV2GetPingServiceEndpointsComplete OnComplete;

	UPROPERTY()
	UHathoraSDKDiscoveryV2 *HathoraSDKDiscoveryV2;
};