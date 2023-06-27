// Copyright 2023 Hathora, Inc.
#include "HathoraPing.h"
#include "DiscoveredPingEndpoint.h"
#include "EditorViewportClient.h"
#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"
#include "HathoraSDK.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include <wchar.h>

UHathoraPing::UHathoraPing(const FObjectInitializer& ObjectInitializer) : UObject(ObjectInitializer)
{
    this->HathoraSdkConfig = NewObject<UHathoraSdkConfig>();
}

void UHathoraPing::GetRegionalPings(const FOnGetRegionalPingsDelegate& OnComplete)
{

	FHttpRequestRef                                                                Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindLambda([&, OnComplete](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) mutable {
		TArray<FDiscoveredPingEndpoint> PingEndpoints;
		if (bSuccess && Response.IsValid() && FJsonObjectConverter::JsonArrayStringToUStruct(Response->GetContentAsString(), &PingEndpoints))
		{
			PingUrlsAndAggregateTimes(PingEndpoints, this->HathoraSdkConfig->GetPingMeasurementsToTake(), OnComplete);
		}
		else
		{
			UE_LOG(LogHathoraSDK, Warning, TEXT("Could not retrieve ping endpoints"));
			TMap<FString, int32> Pings;
			if (!OnComplete.ExecuteIfBound(Pings))
			{
				UE_LOG(LogHathoraSDK, Warning, TEXT("[GetRegionalPings] function pointer was not valid, so OnComplete will not be called"));
			}
		}
	});

    Request->SetURL(FString::Printf(TEXT("%s/discovery/v1/ping"), *this->HathoraSdkConfig->GetBaseUrl()));
	Request->ProcessRequest();
}

// The number of samples should be small so we use the naive median-finding algorithm
inline int32 GetMedianNaively(TArray<int32>& PingMeasurements)
{
			PingMeasurements.Sort();
			int32 NumMeasurements = PingMeasurements.Num();
			int32 MiddleIndex = PingMeasurements.Num() / 2;
			return (NumMeasurements % 2) ? FMath::RoundToInt32((PingMeasurements[MiddleIndex -1] + PingMeasurements[MiddleIndex]) / 2) : PingMeasurements[MiddleIndex];
}

void UHathoraPing::GetMedianPingPerRegion(TMap<FString, TArray<int32>>& AllPingMeasurementsByRegion, const FOnGetRegionalPingsDelegate& OnComplete)
{
		TSharedPtr<TMap<FString, int32>> MedianPingByRegion = MakeShared<TMap<FString, int32>>();
		for (auto& [Region, PingMeasurements]: AllPingMeasurementsByRegion)
		{
			MedianPingByRegion->Add(Region, GetMedianNaively(PingMeasurements));
		}
		(void)OnComplete.ExecuteIfBound(*MedianPingByRegion);
}


inline void InitializePingMeasurementsMap(const TArray<FDiscoveredPingEndpoint>& PingEndpoints, TMap<FString, TArray<int32>>& PingMeasurementsByRegion, int32 MeasurementsToTake)
{
	for (const FDiscoveredPingEndpoint& PingEndpoint: PingEndpoints)
	{
		TArray<int32> Arr = TArray<int32>();
		// Pre-allocate the array to be large enough for the total number of measurements we will take
		// This will avoid re-allocations during the ping checking, which could be subject to race conditions due to the asynchronous
		// nature of the checker
		Arr.Reserve(MeasurementsToTake);
		PingMeasurementsByRegion.Emplace(PingEndpoint.Region, Arr);		
	}	
}

void UHathoraPing::PingUrlsAndAggregateTimes(
	const TArray<FDiscoveredPingEndpoint>& PingEndpoints,
	int32 MeasurementsToTake,
	const FOnGetRegionalPingsDelegate& OnComplete)
{
	TSharedPtr<TMap<FString, TArray<int32>>> Pings = MakeShared<TMap<FString, TArray<int32>>>();
	TSharedPtr<int32>                CompletedRegions = MakeShared<int32>(0);

	InitializePingMeasurementsMap(PingEndpoints, *Pings, MeasurementsToTake);
	
	const int32 RegionsToPing = PingEndpoints.Num();

	// aggregate the results of N asynchronous operations into a single TMap.
	for (const FDiscoveredPingEndpoint& PingEndpoint : PingEndpoints)
	{
		for (int32 PingAttempt = 0; PingAttempt < MeasurementsToTake; ++PingAttempt)
		{
			GetPingTime(PingEndpoint, PingAttempt, FOnGetPingDelegate::CreateLambda([ PingAttempt, MeasurementsToTake, PingEndpoint, CompletedRegions, Pings, RegionsToPing, OnComplete](int32 PingTime, bool bWasSuccesful) {
				if (bWasSuccesful)
				{
					UE_LOG(LogHathoraSDK, Log, TEXT("Ping to %s (%s:%d) took: %d ms"), *PingEndpoint.Region, *PingEndpoint.Host, PingEndpoint.Port, PingTime);
					TArray<int32>* CompletedPingsForRegion = Pings->Find(PingEndpoint.Region);
					CompletedPingsForRegion->Add(PingTime);
				}
				// Regardless of whether the ping was successful, we will mark it complete.
				if (++(*CompletedRegions) == RegionsToPing)
				{
					UE_LOG(LogHathoraSDK, Log, TEXT("all pings complete"));
					(void)OnComplete.ExecuteIfBound(*Pings);
				}
			}));
		}
	}
}


DECLARE_DELEGATE_TwoParams(FOnGetPingDelegate, int32 /* Ping */, bool /* bWasSuccessful */);

void UHathoraPing::GetPingTime(const FDiscoveredPingEndpoint& PingEndpoint, int32 PingID, const FOnGetPingDelegate& OnComplete) 
{
	const FString& MessageTemplate = TEXT("PING-%d");
	const FString& Url = FString::Printf(TEXT("wss://%s:%d/ws"), *PingEndpoint.Host, PingEndpoint.Port);

	// Unfortunately, we can't nest the OnMessage handler inside OnConnected,
	TSharedPtr<double> StartTime = MakeShared<double>(0.0);
	TSharedPtr<TArray<double>> StartTimes = MakeShared<TArray<double>>();
	StartTimes->SetNumZeroed(PingID);
	TSharedPtr<IWebSocket> WebSocket = FWebSocketsModule::Get().CreateWebSocket(Url);

	WebSocket->OnConnectionError().AddLambda([OnComplete](const FString& Reason) {
		UE_LOG(LogHathoraSDK, Warning, TEXT("failed to connect to ping server due to %s"), *Reason);
		(void)OnComplete.ExecuteIfBound(0, false);
	});

	WebSocket->OnMessage().AddLambda([MessageTemplate, WebSocket, StartTime, OnComplete](const FString& Message) {
		if (Message.StartsWith("PING-"))
		{
			TArray<FString> Split;
			Message.ParseIntoArray(Split, *Message, true);
			if (Split.Num() == 2 && Split[0] == "PING")
			{
				int32 PingID = FCString::Atoi(*Split[1]);
			}
		}
		if (StartTime.IsValid() && *StartTime != 0.0 && Message == MessageTemplate)
		{
			const int32 PingTimeMs = static_cast<int32>((FPlatformTime::Seconds() - *StartTime) * 1000);
			(void)OnComplete.ExecuteIfBound(PingTimeMs, true);
			WebSocket->Close();
		}
	});
	
	WebSocket->OnClosed().AddLambda([](int32 StatusCode, const FString& Reason, bool bWasClean) {
		UE_LOG(LogHathoraSDK, VeryVerbose, TEXT("websocket closed %s with status code %d because %s"),
			bWasClean ? TEXT("cleanly") : TEXT("uncleanly"), StatusCode, *Reason); 
	});
	
	WebSocket->OnConnected().AddLambda([PingID, StartTime, StartTimes, WebSocket, MessageTemplate, Url]() {
		UE_LOG(LogHathoraSDK, VeryVerbose, TEXT("websocket connection to %s established"), *Url);

		for (int32 PingAttempt = 0; PingAttempt < PingID; ++PingAttempt)
		{
			(*StartTimes)[PingAttempt] = FPlatformTime::Seconds();
			WebSocket->Send(FString::Printf(MessageTemplate, PingAttempt));
		}
	});
	
	WebSocket->Connect();
}
