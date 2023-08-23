// Copyright 2023 Hathora, Inc. All Rights Reserved.

#include "HathoraSDK.h"

#include "Ssl.h"

// Work around a conflict between a UI namespace defined by engine code and a typedef in OpenSSL
#define UI UI_ST
// Work around assertion macros in ue4
#undef verify
THIRD_PARTY_INCLUDES_START
#include "libwebsockets.h"
THIRD_PARTY_INCLUDES_END
#undef UI

struct CollectPingState {
	bool PingSent;
	bool Disconnected;
	double StartTime;
	int32 PingTimeMs;
};

#define LOCTEXT_NAMESPACE "FHathoraSDKModule"

lws* CollectRegionPingConnect(lws_context* context, const std::string& host, int port, CollectPingState* state)
{
	struct lws_client_connect_info i;

	memset(&i, 0, sizeof(i));

	i.context = context;
	i.port = port;
	i.address = host.c_str();
	i.path = "/ws";
	i.host = i.address;
	i.origin = i.address;
	i.ssl_connection = LCCSCF_USE_SSL;
	i.protocol = "";
	i.local_protocol_name = "";
	i.userdata = state;

	return lws_client_connect_via_info(&i);
}

int CollectRegionPingsCallback(lws* Connection, lws_callback_reasons Reason, void* UserData, void* Data, size_t Length)
{
	auto state = reinterpret_cast<CollectPingState*>(UserData);

	switch (Reason) {

	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
	{
		FUTF8ToTCHAR Convert((const ANSICHAR*)Data, Length);
		FString CloseReasonString(Convert.Length(), Convert.Get());
		UE_LOG(LogHathoraSDK, VeryVerbose, TEXT("LWS_CALLBACK_CLIENT_CONNECTION_ERROR, Reason=%s"), *CloseReasonString);
		state->Disconnected = true;
		return -1;
	}

	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		UE_LOG(LogHathoraSDK, VeryVerbose, TEXT("LWS_CALLBACK_CLIENT_ESTABLISHED"));
		lws_callback_on_writable(Connection);
		break;

	case LWS_CALLBACK_CLIENT_WRITEABLE:
		if (!state->PingSent)
		{
			uint8_t ping[LWS_PRE + 125];

			state->PingSent = true;
			state->StartTime = FPlatformTime::Seconds();

			int toSend = lws_snprintf(reinterpret_cast<char*>(ping) + LWS_PRE, 125, "PING");
			UE_LOG(LogHathoraSDK, VeryVerbose, TEXT("Sending PING"));

			int sent = lws_write(Connection, ping + LWS_PRE, toSend, LWS_WRITE_PING);
			if (sent < toSend)
			{
				UE_LOG(LogHathoraSDK, VeryVerbose, TEXT("Send PING failed %d"), sent);
				return -1;
			}

			lws_callback_on_writable(Connection);
		}
		break;

	case LWS_CALLBACK_CLIENT_CLOSED:
		UE_LOG(LogHathoraSDK, VeryVerbose, TEXT("LWS_CALLBACK_CLIENT_CLOSED"));
		state->Disconnected = true;
		return -1;

	case LWS_CALLBACK_CLIENT_RECEIVE_PONG:
		UE_LOG(LogHathoraSDK, VeryVerbose, TEXT("LWS_CALLBACK_CLIENT_RECEIVE_PONG"));
		state->PingTimeMs = static_cast<int32>((FPlatformTime::Seconds() - state->StartTime) * 1000);
		state->Disconnected = true;
		return -1;

	case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS:
	case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS:
	{
		FSslModule::Get().GetCertificateManager().AddCertificatesToSslContext(static_cast<SSL_CTX*>(UserData));
		return 0;
	}
	case LWS_CALLBACK_OPENSSL_PERFORM_SERVER_CERT_VERIFICATION:
	{
		Data = UserData;
		break;
	}

	default:
		break;
	}

	return lws_callback_http_dummy(Connection, Reason, UserData, Data, Length);
}

int CollectRegionPing(const FDiscoveredPingEndpoint& Endpoint)
{
	const lws_protocols protocols[] =
	{
		{
			"",
			CollectRegionPingsCallback,
			0,
			0,
		},
		{ NULL, NULL, 0, 0 }
	};

	struct lws_context_creation_info info;
	memset(&info, 0, sizeof(info));
	info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
	info.port = CONTEXT_PORT_NO_LISTEN;
	info.protocols = protocols;

	lws_context* context = lws_create_context(&info);
	if (!context)
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("lws init failed"));
		return -1;
	}

	CollectPingState state;
	memset(&state, 0, sizeof(state));

	std::string host(TCHAR_TO_UTF8(*Endpoint.Host));
	lws* connection = CollectRegionPingConnect(context, host, Endpoint.Port, &state);

	while (!state.Disconnected)
	{
		lws_service(context, 1000);
	}
	lws_context_destroy(context);

	UE_LOG(LogHathoraSDK, Log, TEXT("Ping to %s (%s:%d) took: %d ms"), *Endpoint.Region, *Endpoint.Host, Endpoint.Port, state.PingTimeMs);

	return state.PingTimeMs;
}

#undef LOCTEXT_NAMESPACE
