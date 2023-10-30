// Copyright 2023 Hathora, Inc.

#include "HathoraSDKDiscoveryV1.h"
#include "HathoraSDKModule.h"
#include "HathoraSDKConfig.h"
#include "JsonObjectConverter.h"
#include "Icmp.h"

void UHathoraSDKDiscoveryV1::GetPingServiceEndpoints(const FHathoraOnGetPingServiceEndpoints& OnComplete)
{
	SendRequest(
		TEXT("GET"),
		TEXT("/discovery/v1/ping"),
		[&, OnComplete](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) mutable
	{
		TArray<FHathoraDiscoveredPingEndpoint> PingEndpoints;
		if (bSuccess && Response.IsValid())
		{
			FJsonObjectConverter::JsonArrayStringToUStruct(Response->GetContentAsString(), &PingEndpoints);
		}
		else
		{
			UE_LOG(LogHathoraSDK, Warning, TEXT("Could not retrieve ping endpoints"));
		}

		if (!OnComplete.ExecuteIfBound(PingEndpoints))
		{
			UE_LOG(LogHathoraSDK, Warning, TEXT("[GetRegionalPings] function pointer was not valid, so OnComplete will not be called"));
		}
	});
}

void UHathoraSDKDiscoveryV1::GetRegionalPings(const FHathoraOnGetRegionalPings& OnComplete)
{
	OnGetRegionalPingsComplete = OnComplete;
	FHathoraOnGetPingServiceEndpoints OnGetEndpointsComplete;
	OnGetEndpointsComplete.BindDynamic(this, &UHathoraSDKDiscoveryV1::PingUrlsAndAggregateTimes);
	GetPingServiceEndpoints(OnGetEndpointsComplete);
}

void UHathoraSDKDiscoveryV1::PingUrlsAndAggregateTimes(const TArray<FHathoraDiscoveredPingEndpoint>& PingEndpoints)
{
	TSharedPtr<FHathoraRegionPings> AggregateResult = MakeShared<FHathoraRegionPings>();
	TSharedPtr<int32>               CompletedPings = MakeShared<int32>(0);

	const int32 PingsToComplete = PingEndpoints.Num();

	// aggregate the results of N asynchronous operations into a single TMap.
	for (const FHathoraDiscoveredPingEndpoint& PingEndpoint : PingEndpoints)
	{
		FIcmp::IcmpEcho(
			PingEndpoint.Host,
			GetDefault<UHathoraSDKConfig>()->GetPingTimeoutSeconds(),
			[&, AggregateResult, PingEndpoint, CompletedPings, PingsToComplete](FIcmpEchoResult Result)
			{
				if (Result.Status == EIcmpResponseStatus::Success)
				{
					int32 PingTime = Result.Time * 1000;
					UE_LOG(LogHathoraSDK, Log, TEXT("Ping to %s (%s) took: %d ms"), *PingEndpoint.Region, *PingEndpoint.Host, PingTime);
					AggregateResult->Pings.Add(PingEndpoint.Region, PingTime);
				}
				else
				{
					UE_LOG(LogHathoraSDK, Warning, TEXT("Ping to %s (%s) failed: %s"), *PingEndpoint.Region, *PingEndpoint.Host, LexToString(Result.Status));
				}

				// Regardless of whether the ping was successful, we will mark it complete.
				if (++(*CompletedPings) == PingsToComplete)
				{
					UE_LOG(LogHathoraSDK, Log, TEXT("Pings to all Hathora regions complete."));
					(void)OnGetRegionalPingsComplete.ExecuteIfBound(*AggregateResult);
				}
			}
		);
	}
}