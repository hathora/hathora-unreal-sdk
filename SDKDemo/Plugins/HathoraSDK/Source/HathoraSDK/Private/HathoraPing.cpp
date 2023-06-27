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
			PingUrlsAndAggregateTimes(PingEndpoints, OnComplete);
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


void UHathoraPing::PingUrlsAndAggregateTimes(
	const TArray<FDiscoveredPingEndpoint>& PingEndpoints, const FOnGetRegionalPingsDelegate& OnComplete)
{
	TSharedPtr<TMap<FString, int32>> Pings = MakeShared<TMap<FString, int32>>();
	TSharedPtr<int32>                CompletedPings = MakeShared<int32>(0);
	
	const int32 PingsToComplete = PingEndpoints.Num();

	// aggregate the results of N asynchronous operations into a single TMap.
	for (const FDiscoveredPingEndpoint& PingEndpoint : PingEndpoints)
	{

		GetPingTime(PingEndpoint, FOnGetPingDelegate::CreateLambda([ PingEndpoint, CompletedPings, Pings, PingsToComplete, OnComplete](int32 PingTime, bool bWasSuccesful) {
			if (bWasSuccesful)
			{
				UE_LOG(LogHathoraSDK, Log, TEXT("Ping to %s (%s:%d) took: %d ms"), *PingEndpoint.Region, *PingEndpoint.Host, PingEndpoint.Port, PingTime);
				Pings->Add(PingEndpoint.Region, PingTime);
			}
			// Regardless of whether the ping was successful, we will mark it complete.
			if (++(*CompletedPings) == PingsToComplete)
			{
				UE_LOG(LogHathoraSDK, Log, TEXT("Pings to all Hathora regions complete."));
				(void)OnComplete.ExecuteIfBound(*Pings);
			}
		}));
	}
}


DECLARE_DELEGATE_TwoParams(FOnGetPingDelegate, int32 /* Ping */, bool /* bWasSuccessful */);

void UHathoraPing::GetPingTime(const FDiscoveredPingEndpoint& PingEndpoint, const FOnGetPingDelegate& OnComplete) 
{
	const FString& MessageText = TEXT("PING");
	const FString& Url = FString::Printf(TEXT("wss://%s:%d/ws"), *PingEndpoint.Host, PingEndpoint.Port);

	// Unfortunately, we can't nest the OnMessage handler inside OnConnected,
	TSharedPtr<double> StartTime = MakeShared<double>(0.0);
	TSharedPtr<IWebSocket> WebSocket = FWebSocketsModule::Get().CreateWebSocket(Url);

	WebSocket->OnConnectionError().AddLambda([OnComplete](const FString& Reason) {
		UE_LOG(LogHathoraSDK, Warning, TEXT("failed to connect to ping server due to %s"), *Reason);
		(void)OnComplete.ExecuteIfBound(0, false);
	});

	WebSocket->OnMessage().AddLambda([MessageText, WebSocket, StartTime, OnComplete](const FString& Message) {
		if (StartTime.IsValid() && *StartTime != 0.0 && Message == MessageText)
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
	
	WebSocket->OnConnected().AddLambda([StartTime, WebSocket, MessageText, Url]() {
		UE_LOG(LogHathoraSDK, VeryVerbose, TEXT("websocket connection to %s established"), *Url);
		*StartTime = FPlatformTime::Seconds();
		WebSocket->Send(MessageText);
	});
	
	WebSocket->Connect();
}
