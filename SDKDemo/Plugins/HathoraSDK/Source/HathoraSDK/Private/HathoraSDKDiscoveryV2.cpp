// Copyright 2023 Hathora, Inc.

#include "HathoraSDKDiscoveryV2.h"
#include "HathoraSDK.h"
#include "HathoraSDKModule.h"

void UHathoraSDKDiscoveryV2::GetPingServiceEndpoints(const FHathoraOnGetPingServiceEndpoints& OnComplete)
{
	TArray<FHathoraDiscoveredPingEndpoint> PingEndpointsResult;
	TMap<FString, FString> RegionUrls = UHathoraSDK::GetRegionMap();

	for (const auto& Region : RegionUrls)
	{
		FHathoraDiscoveredPingEndpoint Endpoint;
		Endpoint.Region = Region.Key;

		bool bHasPort = Region.Value.Contains(":");

		Endpoint.Host = bHasPort ? Region.Value.LeftChop(Region.Value.Find(":")) : Region.Value;
		Endpoint.Port = bHasPort ? FCString::Atoi(*Region.Value.RightChop(Region.Value.Find(":") + 1)) : 443;
		PingEndpointsResult.Add(Endpoint);
	}

	if (!OnComplete.ExecuteIfBound(PingEndpointsResult))
	{
		UE_LOG(LogHathoraSDK, Warning, TEXT("[GetPingServiceEndpoints] function pointer was not valid, so OnComplete will not be called"));
	}
}

void UHathoraSDKDiscoveryV2::GetRegionalPings(const FHathoraOnGetRegionalPings& OnComplete, int32 InNumPingsPerRegion)
{
	if (InNumPingsPerRegion <= 0)
	{
		UE_LOG(LogHathoraSDK, Warning, TEXT("Cannot ping a region less than 1 time; defaulting to 3 pings per region."));
		InNumPingsPerRegion = 3;
	}

	UHathoraSDK::GetPingsForRegions(
		UHathoraSDK::GetRegionMap(),
		EHathoraPingType::ICMP,
		OnComplete,
		InNumPingsPerRegion
	);
}
