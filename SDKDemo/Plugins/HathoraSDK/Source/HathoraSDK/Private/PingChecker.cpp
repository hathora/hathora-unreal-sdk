// Copyright 2023 Hathora, Inc.


#include "PingChecker.h"

#include "HathoraSDK.h"
#include "HAL/ThreadManager.h"
#include "WebSocketsModule.h"

FPingChecker::FPingChecker(const FDiscoveredPingEndpoint& PingEndpoint, const int32 MeasurementsToTake, const FOnPingCompletionDelegate& OnComplete)
{
	this->PingEndpoint = MakeShared<FDiscoveredPingEndpoint>(PingEndpoint);
	this->MeasurementsToTake = MeasurementsToTake;
	this->OnComplete = OnComplete;
}


inline double GetMedianNaively(TArray<double>& PingMeasurements)
{
	// This mutates PingMeasurements, but since this array is not exposed outside of the context of getting ping measurements
	// it's currently fine.
	PingMeasurements.Sort();
	int32 NumMeasurements = PingMeasurements.Num();
	int32 MiddleIndex = PingMeasurements.Num() / 2;
	return (NumMeasurements % 2) ? PingMeasurements[MiddleIndex] : (PingMeasurements[MiddleIndex - 1] + PingMeasurements[MiddleIndex]) / 2;
}

inline void LogPingMeasurements(const FString& Region, TArray<double>& Measurements)
{
	FString Formatted = "";
	for (const auto& Var : Measurements) {
		Formatted += FString::Printf(TEXT("%.2f ms, "), Var);
	}
	UE_LOG(LogHathoraSDK, Log, TEXT("Ping measurements to %s: %s"), *Region, *Formatted);
}

uint32 FPingChecker::Run()
{
	const FString& MessageText = TEXT("PING");
	const FString Url = FString::Printf(TEXT("wss://%s:%d/ws"), *PingEndpoint->Host, PingEndpoint->Port);
	Socket = FWebSocketsModule::Get().CreateWebSocket(Url);

	UE_LOG(LogHathoraSDK, Log, TEXT("Hello from %s!"), *FThreadManager::Get().GetThreadName(FPlatformTLS::GetCurrentThreadId()));

	// Unfortunately, we can't nest the OnMessage handler inside OnConnected,

	Socket->OnConnectionError().AddLambda([&](const FString& Reason) {
		UE_LOG(LogHathoraSDK, Warning, TEXT("Failed to connect to ping server in %s due to %s"), *PingEndpoint->Region, *Reason);
		(void)OnComplete.ExecuteIfBound(0, false);
	});

	Socket->OnMessage().AddLambda([this, MessageText](const FString& Message ) mutable {
		if (Message == MessageText)
		{
			// Convert s -> ms
			PingResults.Add((FPlatformTime::Seconds() - LastPingStartTime) * 1000);
			
			// Ensure that all measurements happen sequentially by only sending the next message after the previous one has been received.
			if (++MeasurementsTaken == MeasurementsToTake)
			{
				Socket->Close();
				const double MedianPing = GetMedianNaively(PingResults);
				LogPingMeasurements(PingEndpoint->Region, PingResults);
				UE_LOG(LogHathoraSDK, Log, TEXT("ping to %s from thread %s: %.1f ms"), *PingEndpoint->Region,
					*FThreadManager::Get().GetThreadName(FPlatformTLS::GetCurrentThreadId()), MedianPing)
				(void)OnComplete.ExecuteIfBound(MedianPing, true);
			}
			else
			{
				LastPingStartTime = FPlatformTime::Seconds();
				Socket->Send(MessageText);
			}
		}
	});

	Socket->OnClosed().AddLambda([](int32 StatusCode, const FString& Reason, bool bWasClean) {
		UE_LOG(LogHathoraSDK, Verbose, TEXT("Websocket closed %s with status code %d because %s"),
			bWasClean ? TEXT("cleanly") : TEXT("uncleanly"), StatusCode, *Reason);
	});

	Socket->OnConnected().AddLambda([this, MessageText]() {
		UE_LOG(LogHathoraSDK, Verbose, TEXT("Websocket connection to %s established"), *PingEndpoint->Region);
		LastPingStartTime = FPlatformTime::Seconds();
		Socket->Send(MessageText);
	});

	Socket->Connect();
	// TODO what is this return value used for?
	return 0;
}
