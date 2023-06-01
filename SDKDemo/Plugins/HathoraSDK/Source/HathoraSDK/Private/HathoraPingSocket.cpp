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
	// const FString& Url = FString::Printf(TEXT("wss://%s:%d/ws"), *this->PingEndpoint->Host, this->PingEndpoint->Port);
	// const FString& Url = FString::Printf(TEXT("ws://websockets.chilkat.io/wsChilkatEcho.ashx")) ;
	const FString& Url = FString::Printf(TEXT("ws://localhost:8080/ws")) ;
	
	TSharedPtr<IWebSocket> WebSocket = FWebSocketsModule::Get().CreateWebSocket(Url);
	WebSocket->OnConnectionError().AddStatic(&OnConnectionError);
	WebSocket->OnConnected().AddLambda([ WebSocket, OnComplete]() {
		UE_LOG(LogTemp, Display, TEXT("WebSocket is connected..."));
		double CurrentTime = FPlatformTime::Seconds();
		
		WebSocket->OnMessage().AddLambda([=,CurrentTime, OnComplete](const FString& Message) {
			UE_LOG(LogTemp, Display, TEXT("received message: %s"), *Message);
			const int32 PingTimeMs = static_cast<int32>((FPlatformTime::Seconds() - CurrentTime) * 1000);
			if (!OnComplete.ExecuteIfBound(PingTimeMs, true))
			{
				UE_LOG(LogTemp, Warning, TEXT("ping completion delegate was not bound"));
			}
			// WebSocket->Close();
		});

		WebSocket->OnMessageSent().AddLambda([](const FString & Message) {
			UE_LOG(LogTemp, Display, TEXT("%s"), *Message)
		});

		WebSocket->OnClosed().AddLambda([](int32 StatusCode, const FString& Reason, bool bWasClean) {
			UE_LOG(LogTemp, Display, TEXT("websocket was closed!"));
		});
		
		WebSocket->Send(FString::Printf(TEXT("%f"), CurrentTime));
		UE_LOG(LogTemp, Display, TEXT("sent a message "));
	});
	WebSocket->Connect();
}
