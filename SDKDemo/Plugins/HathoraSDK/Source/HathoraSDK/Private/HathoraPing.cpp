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
inline double GetMedianNaively(TArray<double>& PingMeasurements)
{
			PingMeasurements.Sort();
			int32 NumMeasurements = PingMeasurements.Num();
			int32 MiddleIndex = PingMeasurements.Num() / 2;
			return (NumMeasurements % 2) ? (PingMeasurements[MiddleIndex -1] + PingMeasurements[MiddleIndex]) / 2 : PingMeasurements[MiddleIndex];
}

void UHathoraPing::PingUrlsAndAggregateTimes(
	const TArray<FDiscoveredPingEndpoint>& PingEndpoints,
	int32 MeasurementsToTake,
	const FOnGetRegionalPingsDelegate& OnComplete)
{
	TSharedPtr<TMap<FString, int32>> Pings = MakeShared<TMap<FString, int32>>();
	TSharedPtr<int32>                CompletedRegions = MakeShared<int32>(0);

	const int32 RegionsToPing = PingEndpoints.Num();

	// aggregate the results of N asynchronous operations into a single TMap.
	for (const FDiscoveredPingEndpoint& PingEndpoint : PingEndpoints)
	{
			GetPingMeasurements(PingEndpoint, MeasurementsToTake, FOnGetPingMeasurementsDelegate::CreateLambda([ PingEndpoint, CompletedRegions, Pings, RegionsToPing, OnComplete](TArray<double>& PingMeasurements, bool bWasSuccesful) {
				if (bWasSuccesful)
				{
					int32 MedianPing = FMath::RoundToInt32(GetMedianNaively(PingMeasurements));
					UE_LOG(LogHathoraSDK, Log, TEXT("Median ping to %s (%s:%d) took: %d ms"), *PingEndpoint.Region, *PingEndpoint.Host, PingEndpoint.Port, MedianPing);
					Pings->Add(PingEndpoint.Region, MedianPing);
				}
				// Regardless of whether the ping was successful, we will mark it complete.
				if (++(*CompletedRegions) == RegionsToPing)
				{
					UE_LOG(LogHathoraSDK, Log, TEXT("Pings to all Hathora regions complete."));
					(void)OnComplete.ExecuteIfBound(*Pings);
				}
			}));
	}
}

void UHathoraPing::GetPingMeasurements(const FDiscoveredPingEndpoint& PingEndpoint, int32 MeasurementsToTake, const FOnGetPingMeasurementsDelegate& OnComplete) 
{
	const FString& MessageTemplate = TEXT("PING-%d");
	const FString& Url = FString::Printf(TEXT("wss://%s:%d/ws"), *PingEndpoint.Host, PingEndpoint.Port);

	// Unfortunately, we can't nest the OnMessage handler inside OnConnected,
	TSharedPtr<int32> MeasurementsTaken = MakeShared<int32>(0);
	TSharedPtr<TArray<double>> StartTimes = MakeShared<TArray<double>>();
	StartTimes->SetNumZeroed(MeasurementsToTake);
	TSharedPtr<TArray<double>> Measurements = MakeShared<TArray<double>>();
	Measurements->SetNum(MeasurementsToTake);
	TSharedPtr<IWebSocket> WebSocket = FWebSocketsModule::Get().CreateWebSocket(Url);

	WebSocket->OnConnectionError().AddLambda([OnComplete, Measurements](const FString& Reason) {
		UE_LOG(LogHathoraSDK, Warning, TEXT("failed to connect to ping server due to %s"), *Reason);
		(void)OnComplete.ExecuteIfBound(*Measurements, false);
	});

	WebSocket->OnMessage().AddLambda([WebSocket, PingEndpoint, MeasurementsTaken, MeasurementsToTake, Measurements, StartTimes, OnComplete](const FString& Message) {
		UE_LOG(LogHathoraSDK, Display, TEXT("received message %s from %s"), *Message, *PingEndpoint.Region);
		if (Message.StartsWith("PING-"))
		{
			TArray<FString> Split;
			Message.ParseIntoArray(Split, TEXT("-"), true);
			if (Split.Num() == 2 && Split[0] == "PING")
			{
				if (const int PingID = FCString::Strtoi(*Split[1], nullptr, 10); PingID > 0 && PingID <= MeasurementsToTake)
				{
					// Convert s -> ms
					(*Measurements)[PingID - 1] = (FPlatformTime::Seconds() - (*StartTimes)[PingID - 1]) * 1000;
					UE_LOG(LogHathoraSDK, Display, TEXT("ping %d/%d to %s took %f ms"), PingID, MeasurementsToTake, *PingEndpoint.Region, (*Measurements)[PingID - 1]);
				}
			}
		}

		if (Measurements.IsValid() && MeasurementsTaken.IsValid() && ++(*MeasurementsTaken) == MeasurementsToTake)
		{
			WebSocket->Close();
			(void)OnComplete.ExecuteIfBound(*Measurements, true);
		}
	});
	
	WebSocket->OnClosed().AddLambda([](int32 StatusCode, const FString& Reason, bool bWasClean) {
		UE_LOG(LogHathoraSDK, VeryVerbose, TEXT("websocket closed %s with status code %d because %s"),
			bWasClean ? TEXT("cleanly") : TEXT("uncleanly"), StatusCode, *Reason); 
	});
	
	WebSocket->OnConnected().AddLambda([MeasurementsToTake, StartTimes, WebSocket, MessageTemplate, Url]() {
		UE_LOG(LogHathoraSDK, VeryVerbose, TEXT("websocket connection to %s established"), *Url);

		// Use 1-based attempt numbers to avoid overloading 0-based error condition for Strtoi
		for (int32 PingAttempt = 1; PingAttempt <= MeasurementsToTake; ++PingAttempt)
		{
			(*StartTimes)[PingAttempt - 1] = FPlatformTime::Seconds();
			WebSocket->Send(FString::Printf(TEXT("PING-%d"), PingAttempt));
			UE_LOG(LogHathoraSDK, Display, TEXT("sent message %d/%d to %s"), PingAttempt, MeasurementsToTake, *Url);
		}
	});
	
	WebSocket->Connect();
}
