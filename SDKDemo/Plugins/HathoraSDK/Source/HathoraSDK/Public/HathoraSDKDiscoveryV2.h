// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "HathoraSDKAPI.h"
#include "HathoraTypes.h"
#include "HathoraSDKDiscoveryV2.generated.h"

UCLASS(BlueprintType)
class HATHORASDK_API UHathoraSDKDiscoveryV2 : public UHathoraSDKAPI
{
	GENERATED_BODY()

public:
	// Get ping times to all available Hathora Cloud regions.
	// Each region is pinged NumPingsPerRegion times and the
	// minimum is returned.
	// Pings are returned in milliseconds.
	// @param OnComplete The delegate to call when the request is complete with averaged ping times.
	// @param NumPingsPerRegion The number of pings to send to each region.
	void GetRegionalPings(const FHathoraOnGetRegionalPings& OnComplete, int32 NumPingsPerRegion = 3);

	typedef TDelegate<void(const TArray<FHathoraDiscoveredPingEndpoint>&)> FHathoraOnGetPingServiceEndpoints;

	// Returns an array of all regions with a host that a client can ping via ICMP.
	// See the "Get Regional Pings" functions that will handle the full ping process for you.
	// @param OnComplete The delegate to call when the request is complete.
	void GetPingServiceEndpoints(const FHathoraOnGetPingServiceEndpoints& OnComplete);

private:
	FHathoraOnGetRegionalPings OnGetRegionalPingsComplete;
	TArray<FHathoraDiscoveredPingEndpoint> PingEndpoints;
	int32 NumPingsPerRegion;
	int32 NumPingsPerRegionCompleted;
	TSharedPtr<TMap<FString, TArray<int32>>> PingResults;

	void PingEachRegion();
};
