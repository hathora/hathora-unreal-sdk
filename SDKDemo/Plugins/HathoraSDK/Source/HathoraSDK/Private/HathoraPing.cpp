// Copyright 2023 Hathora, Inc.
#include "HathoraPing.h"
#include "DiscoveredPingEndpoint.h"
#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"
#include "HathoraSDK.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

UHathoraPing::UHathoraPing(const FObjectInitializer& ObjectInitializer)
	: UObject(ObjectInitializer)
{
	this->HathoraSdkConfig = NewObject<UHathoraSdkConfig>();
}

inline void LogPingMeasurements(const FString& Region, TArray<double>& Measurements)
{
	FString Formatted = "";
	for (const auto& Var : Measurements) {
		Formatted += FString::Printf(TEXT("%.1f ms, "), Var);
	}
	UE_LOG(LogHathoraSDK, Log, TEXT("Ping measurements to %s: %s"), *Region, *Formatted);
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
	// This mutates PingMeasurements, but since this array is not exposed outside of the context of getting ping measurements
	// it's currently fine.
	PingMeasurements.Sort();
	int32 NumMeasurements = PingMeasurements.Num();
	int32 MiddleIndex = PingMeasurements.Num() / 2;
	return (NumMeasurements % 2) ? PingMeasurements[MiddleIndex] : (PingMeasurements[MiddleIndex - 1] + PingMeasurements[MiddleIndex]) / 2;
}

void UHathoraPing::PingUrlsAndAggregateTimes(
	const TArray<FDiscoveredPingEndpoint>& PingEndpoints,
	int32                                  MeasurementsToTake,
	const FOnGetRegionalPingsDelegate&     OnComplete)
{
	TSharedPtr<TMap<FString, TArray<double>>> Pings = MakeShared<TMap<FString, TArray<double>>>();
	TSharedPtr<TMap<FString, int32>> Results = MakeShared<TMap<FString, int32>>();
	TSharedPtr<int32>                CompletedRegions = MakeShared<int32>(0);

	const int32 RegionsToPing = PingEndpoints.Num();

	// aggregate the results of N asynchronous operations into a single TMap.
	for (const FDiscoveredPingEndpoint& PingEndpoint : PingEndpoints)
	{
		GetPingMeasurements(PingEndpoint, MeasurementsToTake, Pings, FOnGetPingMeasurementsDelegate::CreateLambda([ PingEndpoint, CompletedRegions, Pings, Results, RegionsToPing, OnComplete](bool bWasSuccesful) mutable {
			if (bWasSuccesful)
			{
				LogPingMeasurements(PingEndpoint.Region, *Pings->Find(PingEndpoint.Region));
				const int32 MedianPing = FMath::RoundToInt32(GetMedianNaively(*Pings->Find(PingEndpoint.Region)));
				UE_LOG(LogHathoraSDK, Log, TEXT("Median ping to %s (%s:%d) took: %d ms"), *PingEndpoint.Region, *PingEndpoint.Host, PingEndpoint.Port, MedianPing);
				Results->Add(PingEndpoint.Region, MedianPing);
			}
			// Regardless of whether the ping was successful, we will mark it complete.
			if (++(*CompletedRegions) == RegionsToPing)
			{
				UE_LOG(LogHathoraSDK, Log, TEXT("Pings to all Hathora regions complete."));
				(void)OnComplete.ExecuteIfBound(*Results);
			}
		}));
	}
}

void UHathoraPing::GetPingMeasurements(const FDiscoveredPingEndpoint& PingEndpoint, int32 MeasurementsToTake, TSharedPtr<TMap<FString, TArray<double>>> Pings, const FOnGetPingMeasurementsDelegate& OnComplete)
{
	const FString& MessageText = TEXT("PING");
	const FString& Url = FString::Printf(TEXT("wss://%s:%d/ws"), *PingEndpoint.Host, PingEndpoint.Port);

	// Unfortunately, we can't nest the OnMessage handler inside OnConnected,
	TSharedPtr<int32>          MeasurementsTaken = MakeShared<int32>(0);
	TSharedPtr<TArray<double>> StartTimes = MakeShared<TArray<double>>();
	StartTimes->SetNumZeroed(MeasurementsToTake);
	Pings->Emplace(PingEndpoint.Region);
	Pings->Find(PingEndpoint.Region)->SetNumZeroed(MeasurementsToTake);
	TSharedPtr<IWebSocket> WebSocket = FWebSocketsModule::Get().CreateWebSocket(Url);

	WebSocket->OnConnectionError().AddLambda([OnComplete, PingEndpoint](const FString& Reason) {
		UE_LOG(LogHathoraSDK, Warning, TEXT("Failed to connect to ping server in %s due to %s"), *PingEndpoint.Region, *Reason);
		(void)OnComplete.ExecuteIfBound(false);
	});

	WebSocket->OnMessage().AddLambda([WebSocket, PingEndpoint, MessageText, MeasurementsTaken, MeasurementsToTake, Pings, StartTimes, OnComplete](const FString& Message) mutable {
		if (!MeasurementsTaken.IsValid() || !StartTimes.IsValid())
		{
			(void)OnComplete.ExecuteIfBound(false);
		}

		if (Message == MessageText)
		{
			// Convert s -> ms
			(*Pings->Find(PingEndpoint.Region))[*MeasurementsTaken] = (FPlatformTime::Seconds() - (*StartTimes)[*MeasurementsTaken]) * 1000;

			// Ensure that all measurements happen sequentially by only sending the next message after the previous one has been received.
			if (++(*MeasurementsTaken) == MeasurementsToTake)
			{
				WebSocket->Close();
				(void)OnComplete.ExecuteIfBound(true);
			}
			else
			{
				(*StartTimes)[*MeasurementsTaken] = FPlatformTime::Seconds();
				WebSocket->Send(MessageText);
			}
		}
	});

	WebSocket->OnClosed().AddLambda([](int32 StatusCode, const FString& Reason, bool bWasClean) {
		UE_LOG(LogHathoraSDK, Verbose, TEXT("Websocket closed %s with status code %d because %s"),
			bWasClean ? TEXT("cleanly") : TEXT("uncleanly"), StatusCode, *Reason);
	});

	WebSocket->OnConnected().AddLambda([StartTimes, WebSocket, PingEndpoint, MessageText]() {
		UE_LOG(LogHathoraSDK, Verbose, TEXT("Websocket connection to %s established"), *PingEndpoint.Region);
		(*StartTimes)[0] = FPlatformTime::Seconds();
		WebSocket->Send(MessageText);
	});

	WebSocket->Connect();
}
