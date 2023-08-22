// Copyright 2023 Hathora, Inc. All Rights Reserved.

#include "HathoraSDK.h"
#include "HathoraSDK_Ping.h"

#define LOCTEXT_NAMESPACE "FHathoraSDKModule"

void FHathoraSDKModule::GetRegionalPings(TArray<FDiscoveredPingEndpoint> PingEndpoints, const FOnGetRegionalPingsDelegate& OnComplete)
{
	if (PingEndpoints.Num() == 0)
	{
		// Call GetPingServiceEndpoints to get the list of endpoints
		FPingServiceEndpointsDelegate Delegate;
		Delegate.BindLambda([this, OnComplete](TArray<FDiscoveredPingEndpoint> OutPingEndpoints)
		{
			GetRegionalPings_Internal(OutPingEndpoints, OnComplete);
		});
		GetPingServiceEndpoints(Delegate);
		return;
	}

	GetRegionalPings_Internal(PingEndpoints, OnComplete);
}

void FHathoraSDKModule::GetRegionalPings_Internal(TArray<FDiscoveredPingEndpoint> PingEndpoints, const FOnGetRegionalPingsDelegate& OnComplete)
{
	if (PingEndpoints.Num() == 0)
	{
		UE_LOG(LogHathoraSDK, Warning, TEXT("GetRegionalPings_Internal called with an empty list of endpoints"));
		TMap<FString, int32> emptyResult;
		OnComplete.ExecuteIfBound(emptyResult);
		return;
	}

	TSharedPtr<TMap<FString, int32>> pings = MakeShared<TMap<FString, int32>>();
	TSharedPtr<int32>                pendingPings = MakeShared<int32>(PingEndpoints.Num());

	if (PingEndpoints.Num() > 0)
	{
		for (const FDiscoveredPingEndpoint& endpoint : PingEndpoints)
		{
			// Collect the ping on a worker thread
			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [endpoint, pings, pendingPings, OnComplete]()
			{
				int32 ping = CollectRegionPing(endpoint);

				// Post the results back to the main thread
				AsyncTask(ENamedThreads::GameThread, [ping, endpoint, pings, pendingPings, OnComplete]()
				{
					if (ping > 0)
					{
						pings->Add(endpoint.Region, ping);
					}

					*pendingPings -= 1;
					if (*pendingPings <= 0)
					{
						OnComplete.ExecuteIfBound(*pings);
					}
				});
			});
		}
	}
}

#undef LOCTEXT_NAMESPACE
