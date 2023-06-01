#pragma once
#include "DiscoveredPingEndpoint.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"

class FHathoraPingSocket
{
public:

	DECLARE_DELEGATE_TwoParams(FOnGetPingDelegate, int32 /* Ping */, bool /* bWasSuccessful */);
	FHathoraPingSocket(TSharedPtr<FDiscoveredPingEndpoint> PingEndpoint);
	void GetPingTime(const FOnGetPingDelegate& OnComplete) const;

private:
	static void OnMessageReceived(const FString& Message, double TimeMessageSent, const FOnGetPingDelegate& OnComplete);
	TSharedPtr<FDiscoveredPingEndpoint> PingEndpoint;
};
