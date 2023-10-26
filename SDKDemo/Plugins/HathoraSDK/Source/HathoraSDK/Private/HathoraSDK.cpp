// Copyright 2023 Hathora, Inc.

#include "HathoraSDK.h"
#include "HathoraSDKModule.h"
#include "DiscoveredPingEndpoint.h"
#include "Icmp.h"
#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

void UHathoraSDK::GetRegionalPings(const FOnGetRegionalPingsDelegate& OnComplete)
{
	FHttpRequestRef                                                                Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindLambda([&, OnComplete](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) mutable {
		TArray<FDiscoveredPingEndpoint> PingEndpoints;
		if (bSuccess && Response.IsValid() && FJsonObjectConverter::JsonArrayStringToUStruct(Response->GetContentAsString(), &PingEndpoints))
		{
			PingUrlsAndAggregateTimes(PingEndpoints, OnComplete);
		}
		else
		{
			UE_LOG(LogHathoraSDK, Warning, TEXT("Could not retrieve ping endpoints"));
			FHathoraRegionPings Pings;
			if (!OnComplete.ExecuteIfBound(Pings))
			{
				UE_LOG(LogHathoraSDK, Warning, TEXT("[GetRegionalPings] function pointer was not valid, so OnComplete will not be called"));
			}
		}
	});

	Request->SetURL(FString::Printf(TEXT("%s/discovery/v1/ping"), *GetDefault<UHathoraSDKConfig>()->GetBaseUrl()));
	Request->ProcessRequest();
}


void UHathoraSDK::PingUrlsAndAggregateTimes(
	const TArray<FDiscoveredPingEndpoint>& PingEndpoints, const FOnGetRegionalPingsDelegate& OnComplete)
{
	TSharedPtr<FHathoraRegionPings> Result = MakeShared<FHathoraRegionPings>();
	TSharedPtr<int32>               CompletedPings = MakeShared<int32>(0);

	const int32 PingsToComplete = PingEndpoints.Num();

	// aggregate the results of N asynchronous operations into a single TMap.
	for (const FDiscoveredPingEndpoint& PingEndpoint : PingEndpoints)
	{
		GetPingTime(PingEndpoint, FOnGetPingDelegate::CreateLambda([PingEndpoint, CompletedPings, Result, PingsToComplete, OnComplete](int32 PingTime, bool bWasSuccessful) {
			if (bWasSuccessful)
			{
				UE_LOG(LogHathoraSDK, Log, TEXT("Ping to %s (%s:%d) took: %d ms"), *PingEndpoint.Region, *PingEndpoint.Host, PingEndpoint.Port, PingTime);
				Result->Pings.Add(PingEndpoint.Region, PingTime);
			}
			// Regardless of whether the ping was successful, we will mark it complete.
			if (++(*CompletedPings) == PingsToComplete)
			{
				UE_LOG(LogHathoraSDK, Log, TEXT("Pings to all Hathora regions complete."));
				(void)OnComplete.ExecuteIfBound(*Result);
			}
		}));
	}
}


void UHathoraSDK::GetPingTime(const FDiscoveredPingEndpoint& PingEndpoint, const FOnGetPingDelegate& OnComplete)
{
	FIcmp::IcmpEcho(
		PingEndpoint.Host,
		GetDefault<UHathoraSDKConfig>()->GetPingTimeoutMs(),
		[OnComplete](FIcmpEchoResult Result) {
			OnComplete.ExecuteIfBound(Result.Time * 1000, Result.Status == EIcmpResponseStatus::Success);
		}
	);
}
