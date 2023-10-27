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

void UHathoraSDKDiscoveryV1::GetRegionalPings(const FHathoraOnGetRegionalPings& OnComplete, int32 InNumPingsPerRegion)
{
	OnGetRegionalPingsComplete = OnComplete;
	NumPingsPerRegion = InNumPingsPerRegion;
	FHathoraOnGetPingServiceEndpoints OnGetEndpointsComplete;
	OnGetEndpointsComplete.BindDynamic(this, &UHathoraSDKDiscoveryV1::InitiatePings);
	GetPingServiceEndpoints(OnGetEndpointsComplete);
}

void UHathoraSDKDiscoveryV1::InitiatePings(const TArray<FHathoraDiscoveredPingEndpoint>& InPingEndpoints)
{
	PingEndpoints = InPingEndpoints;
	NumPingsPerRegionCompleted = 0;
	PingResults = MakeShared<TMap<FString, TArray<int32>>>();
	PingEachRegion();
}

void UHathoraSDKDiscoveryV1::PingEachRegion()
{
	// recursively call this function until we have pinged each region
	// the desired number of times, and then aggregate the results and return
	// this ensures subsequent pings to a particular region are done sequentially
	// instead of simultaneously
	if (NumPingsPerRegion == NumPingsPerRegionCompleted)
	{
		UE_LOG(LogHathoraSDK, Log, TEXT("Completed multiple pings to each Hathora region; returning the averages."));

		FHathoraRegionPings FinalResult;

		for (const TPair<FString, TArray<int32>>& PingResult : *PingResults)
		{
			int32 Sum = 0;
			for (int32 PingTime : PingResult.Value)
			{
				Sum += PingTime;
			}
			FinalResult.Pings.Add(PingResult.Key, FMath::RoundToInt(float(Sum) / float(PingResult.Value.Num())));
		}

		OnGetRegionalPingsComplete.ExecuteIfBound(FinalResult);

		return;
	}

	NumPingsPerRegionCompleted++;

	TSharedPtr<int32> CompletedPings = MakeShared<int32>(0);
	const int32 PingsToComplete = PingEndpoints.Num();

	// ping each region once
	for (const FHathoraDiscoveredPingEndpoint& PingEndpoint : PingEndpoints)
	{
		FIcmp::IcmpEcho(
			PingEndpoint.Host,
			GetDefault<UHathoraSDKConfig>()->GetPingTimeoutSeconds(),
			[&, PingEndpoint, CompletedPings, PingsToComplete](FIcmpEchoResult Result)
			{
				if (Result.Status == EIcmpResponseStatus::Success)
				{
					int32 PingTime = Result.Time * 1000;
					UE_LOG(LogHathoraSDK, Log, TEXT("Ping to %s (%s) took: %d ms"), *PingEndpoint.Region, *PingEndpoint.Host, PingTime);
					TArray<int32>* RegionPings = PingResults->Find(PingEndpoint.Region);
					if (RegionPings == nullptr)
					{
						TArray<int32> NewRegionPings;
						NewRegionPings.Add(PingTime);
						PingResults->Add(PingEndpoint.Region, NewRegionPings);
					}
					else
					{
						RegionPings->Add(PingTime);
					}
				}
				else
				{
					UE_LOG(LogHathoraSDK, Warning, TEXT("Ping to %s (%s) failed: %s"), *PingEndpoint.Region, *PingEndpoint.Host, LexToString(Result.Status));
				}

				// Regardless of whether the ping was successful, we will mark it complete.
				if (++(*CompletedPings) == PingsToComplete)
				{
					UE_LOG(LogHathoraSDK, Log, TEXT("Pings to each region complete; triggering another set of pings."));
					PingEachRegion();
				}
			}
		);
	}
}