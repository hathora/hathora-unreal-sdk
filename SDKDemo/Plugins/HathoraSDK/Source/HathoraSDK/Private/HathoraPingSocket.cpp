#include "HathoraPingSocket.h"


void OnConnectionError(const FString& Reason)
{
	UE_LOG(LogTemp, Error, TEXT("failed to connect due to %s"), *Reason);
}

FHathoraPingSocket::FHathoraPingSocket(TSharedPtr<FDiscoveredPingEndpoint> PingEndpoint)
{
	this->PingEndpoint = PingEndpoint;
}

void FHathoraPingSocket::GetPingTime(const FOnGetPingDelegate& OnComplete) const
{
	const FString& MessageText = TEXT("PING");
	const FString& Url = FString::Printf(TEXT("wss://%s:%d/ws"), *this->PingEndpoint->Host, this->PingEndpoint->Port);

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
