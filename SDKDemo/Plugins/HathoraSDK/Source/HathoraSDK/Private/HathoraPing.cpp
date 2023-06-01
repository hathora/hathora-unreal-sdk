// Copyright 2023 Hathora, Inc.
#include "HathoraPing.h"
#include "DiscoveredPingEndpoint.h"
#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "HathoraPingSocket.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

void UHathoraPing::GetRegionalPings(const FOnGetRegionalPingsDelegate& OnComplete)
{

	FHttpRequestRef                                                                Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindLambda([&, OnComplete](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) mutable {
		TArray<FDiscoveredPingEndpoint> PingEndpoints;
		TMap<FString, int32>            Pings;
		if (bSuccess && Response.IsValid() && FJsonObjectConverter::JsonArrayStringToUStruct(Response->GetContentAsString(), &PingEndpoints))
		{
			PingUrlsAndAggregateTimes(PingEndpoints, OnComplete);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Could not retrieve ping endpoints"));
		}
		if (!OnComplete.ExecuteIfBound(Pings))
		{
			UE_LOG(LogTemp, Error, TEXT("[GetRegionalPings] function pointer was not valid, so OnComplete will not be called"));
		}
	});

	// TODO - How should this get passed in to this package?
	Request->SetURL("https://api.hathora.dev/discovery/v1/ping");
	Request->ProcessRequest();
}


void UHathoraPing::PingUrlsAndAggregateTimes(
	const TArray<FDiscoveredPingEndpoint>& PingEndpoints, const FOnGetRegionalPingsDelegate& OnComplete)
{
	TMap<FString, int32> Pings;
	int32                CompletedPings = 0;
	const int32 PingsToComplete = PingEndpoints.Num();

	// aggregate the results of N asynchronous operations into a single TMap.
	for (const FDiscoveredPingEndpoint& PingEndpoint : PingEndpoints)
	{
		TUniquePtr<FHathoraPingSocket> PingSocket = MakeUnique<FHathoraPingSocket>(MakeShared<FDiscoveredPingEndpoint>(PingEndpoint));

		PingSocket->GetPingTime(FHathoraPingSocket::FOnGetPingDelegate::CreateLambda([&PingEndpoint, &CompletedPings, &Pings, PingsToComplete, OnComplete](int32 PingTime, bool bWasSuccesful) {
			UE_LOG(LogTemp, Display, TEXT("ping of %s completed, for sure"), *PingEndpoint.Host);
			if (bWasSuccesful)
			{
				UE_LOG(LogTemp, Display, TEXT("ping of %s was successful %d"), *PingEndpoint.Host, PingTime);
				Pings.Add(PingEndpoint.Region, PingTime);
			}
			// Regardless of whether the ping was successful, we will mark it complete.
			// TODO - should we have a time out???
			if (++CompletedPings == PingsToComplete)
			{
				(void)OnComplete.ExecuteIfBound(Pings);
			}
		}));
	}
}
