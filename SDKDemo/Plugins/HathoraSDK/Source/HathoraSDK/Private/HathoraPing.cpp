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
		if (bSuccess && Response.IsValid() && FJsonObjectConverter::JsonArrayStringToUStruct(Response->GetContentAsString(), &PingEndpoints))
		{
			PingUrlsAndAggregateTimes(PingEndpoints, OnComplete);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Could not retrieve ping endpoints"));
			TMap<FString, int32> Pings;
			if (!OnComplete.ExecuteIfBound(Pings))
			{
				UE_LOG(LogTemp, Error, TEXT("[GetRegionalPings] function pointer was not valid, so OnComplete will not be called"));
			}
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
	UE_LOG(LogTemp, Display,TEXT("%d"), Pings.Num());
	int32                CompletedPings = 0;
	const int32 PingsToComplete = PingEndpoints.Num();

	// aggregate the results of N asynchronous operations into a single TMap.
	for (const FDiscoveredPingEndpoint& PingEndpoint : PingEndpoints)
	{

		GetPingTime(PingEndpoint, FHathoraPingSocket::FOnGetPingDelegate::CreateLambda([&, PingEndpoint, CompletedPings, Pings, PingsToComplete, =OnComplete](int32 PingTime, bool bWasSuccesful) {
			UE_LOG(LogTemp, Display, TEXT("ping of %s completed, for sure"), *PingEndpoint.Host);
			if (bWasSuccesful)
			{
				UE_LOG(LogTemp, Display, TEXT("ping of %s was successful %d"), *PingEndpoint.Host, PingTime);
				Pings.Add(PingEndpoint.Region, PingTime);
			}
			// Regardless of whether the ping was successful, we will mark it complete.
			if (++CompletedPings == PingsToComplete)
			{
				(void)OnComplete.ExecuteIfBound(Pings);
			}
		}));
	}
}


DECLARE_DELEGATE_TwoParams(FOnGetPingDelegate, int32 /* Ping */, bool /* bWasSuccessful */);

void UHathoraPing::GetPingTime(const FDiscoveredPingEndpoint& PingEndpoint, const FOnGetPingDelegate& OnComplete) 
{
	const FString& MessageText = TEXT("PING");
	const FString& Url = FString::Printf(TEXT("wss://%s:%d/ws"), *PingEndpoint.Host, PingEndpoint.Port);

	// Unfortunately, we can't nest the OnMessage handler inside OnConnected, so we just use this local variable instead.
	double                 StartTime = 0.0;
	TSharedPtr<IWebSocket> WebSocket = FWebSocketsModule::Get().CreateWebSocket(Url);

	WebSocket->OnConnectionError().AddLambda([OnComplete](const FString& Reason) {
		UE_LOG(LogTemp, Error, TEXT("failed to connect due to %s"), *Reason);
		(void)OnComplete.ExecuteIfBound(0, false);
	});

	WebSocket->OnMessage().AddLambda([MessageText, WebSocket, &StartTime, OnComplete](const FString& Message) {
		UE_LOG(LogTemp, Display, TEXT("received message: %s %d %d"), *Message, Message.Len(), MessageText.Len());

		
		if (StartTime != 0 && Message == MessageText)
		{
			const int32 PingTimeMs = static_cast<int32>((FPlatformTime::Seconds() - StartTime) * 1000);
			(void)OnComplete.ExecuteIfBound(PingTimeMs, true);
			WebSocket->Close();
		}
	});
	
	WebSocket->OnClosed().AddLambda([](int32 StatusCode, const FString& Reason, bool bWasClean) {
		UE_LOG(LogTemp, Display, TEXT("websocket was closed!"));
	});
	
	WebSocket->OnConnected().AddLambda([&StartTime, WebSocket, MessageText]() {
		UE_LOG(LogTemp, Display, TEXT("WebSocket is connected..."));
		StartTime = FPlatformTime::Seconds();
		WebSocket->Send(MessageText);
	});
	
	WebSocket->Connect();
}
