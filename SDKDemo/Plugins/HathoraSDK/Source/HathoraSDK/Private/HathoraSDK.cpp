// Copyright Epic Games, Inc. All Rights Reserved.

#include "HathoraSDK.h"

#include "DiscoveredPingEndpoint.h"

#include "Async/Async.h"
#include "HAL/RunnableThread.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "JsonObjectConverter.h"
#include "Ssl.h"
#include "TimerManager.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#endif

// Work around a conflict between a UI namespace defined by engine code and a typedef in OpenSSL
#define UI UI_ST
// Work around assertion macros in ue4
#undef verify 
THIRD_PARTY_INCLUDES_START
#include "libwebsockets.h"
THIRD_PARTY_INCLUDES_END
#undef UI

#if PLATFORM_WINDOWS
#include "Windows/HideWindowsPlatformTypes.h"
#endif

#include <string>

#define LOCTEXT_NAMESPACE "FHathoraSDKModule"

DEFINE_LOG_CATEGORY(LogHathoraSDK)
void FHathoraSDKModule::StartupModule()
{
	// Load Configuration
	BaseUrl = TEXT("https://api.hathora.dev");
	GConfig->GetString(TEXT("HathoraSDK"), TEXT("BaseUrl"), BaseUrl, GGameIni);
	
	GetConnectionInfoTimeoutSeconds = 90.0f;
	GConfig->GetFloat(TEXT("HathoraSDK"), TEXT("GetConnectionInfoTimeoutSeconds"), GetConnectionInfoTimeoutSeconds, GGameIni);

	GetConnectionInfoDelaysSeconds = { "0.5", "1.0", "2.0", "3.0" };
	GConfig->GetArray(TEXT("HathoraSDK"), TEXT("GetConnectionInfoDelaysSeconds"), GetConnectionInfoDelaysSeconds, GGameIni);
}

void FHathoraSDKModule::ShutdownModule()
{
}

void FHathoraSDKModule::SetRetryManager(TSharedPtr<class FHttpRetrySystem::FManager>& httpRetryManager)
{
	HttpRetryManager = httpRetryManager;
}

TSharedRef<IHttpRequest, ESPMode::ThreadSafe> FHathoraSDKModule::NewRequest()
{
	static const TSet<int32> retryCodes(TArray<int32>({ 400, 500, 501, 502, 503, 504 }));
	static const TSet<FName> retryVerbs(TArray<FName>({ FName(TEXT("GET")), FName(TEXT("HEAD")), FName(TEXT("POST")) }));

	if (!HttpRetryManager.IsValid()) {
		// Fallback to the Http Manager without retries
		return FHttpModule::Get().CreateRequest();
	}

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> result = HttpRetryManager->CreateRequest(
		2,
		FHttpRetrySystem::FRetryTimeoutRelativeSecondsSetting(),
		retryCodes,
		retryVerbs);
	result->SetTimeout(10.0f);

	return result;
}

//
// GetRegionalPings
//

struct CollectPingState {
	bool PingSent;
	bool Disconnected;
	double StartTime;
	int32 PingTimeMs;
};

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

void FHathoraSDKModule::GetRegionalPings(const FOnGetRegionalPingsDelegate& OnComplete)
{
	TSharedRef< IHttpRequest, ESPMode::ThreadSafe > request = NewRequest();
	request->SetURL(FString::Format(TEXT("{0}/discovery/v1/ping"), { BaseUrl }));
	request->OnProcessRequestComplete().BindLambda([OnComplete](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) {
		TArray<FDiscoveredPingEndpoint> endpoints;
		if (bSuccess && Response.IsValid() && FJsonObjectConverter::JsonArrayStringToUStruct(Response->GetContentAsString(), &endpoints))
		{
			TSharedPtr<TMap<FString, int32>> pings = MakeShared<TMap<FString, int32>>();
			TSharedPtr<int32>                pendingPings = MakeShared<int32>(endpoints.Num());

			if (endpoints.Num() > 0)
			{
				for (const FDiscoveredPingEndpoint& endpoint : endpoints)
				{
					// Collect the ping on a worker thread
					AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [endpoint, pings, pendingPings, OnComplete]() 
					{
						int32 ping = CollectRegionPing(endpoint);

						// Post the results back to the main thread
						AsyncTask(ENamedThreads::GameThread, [ping, endpoint, pings, pendingPings, OnComplete]() 
						{
							if (ping > 0)
							{
								pings->Add(endpoint.Region, ping);
							}

							*pendingPings -= 1;
							if (*pendingPings <= 0)
							{
								OnComplete.ExecuteIfBound(*pings);
							}
						});
					});
				}
			}
			else
			{
				UE_LOG(LogHathoraSDK, Warning, TEXT("/discovery/v1/ping returned an empty list of regions"));
				TMap<FString, int32> emptyResult;
				OnComplete.ExecuteIfBound(emptyResult);
			}	
		}
		else
		{
			UE_LOG(LogHathoraSDK, Warning, TEXT("Could not retrieve ping endpoints"));
			TMap<FString, int32> emptyResult;
			OnComplete.ExecuteIfBound(emptyResult);
		}
		});
	request->ProcessRequest();
}

//
// GetConnectionInfo
//

void FHathoraSDKModule::GetConnectionInfo(const FString& AppId, const FString& RoomId, const FConnectionInfoDelegate& OnComplete)
{
	GetConnectionInfo_Internal(AppId, RoomId, FPlatformTime::Seconds(), 0, OnComplete);
}

void FHathoraSDKModule::GetConnectionInfo_Internal(const FString& AppId, const FString& RoomId, double StartTime, int32 RetryCount, const FConnectionInfoDelegate& OnComplete)
{
	TSharedRef< IHttpRequest, ESPMode::ThreadSafe > request = NewRequest();
	request->SetURL(FString::Format(TEXT("{0}/rooms/v2/{1}/connectioninfo/{2}"), { BaseUrl, AppId, RoomId }));

	request->OnProcessRequestComplete().BindLambda([this, AppId, RoomId, StartTime, RetryCount, OnComplete](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) {
		if (!bSuccess || !Response.IsValid())
		{
			UE_LOG(LogHathoraSDK, Warning, TEXT("rooms/v2/connectioninfo failed to connect"));
			OnComplete.ExecuteIfBound(false, FString(), 0);
			return;
		}

		if (Response->GetResponseCode() != 200)
		{
			UE_LOG(LogHathoraSDK, Warning,
				TEXT("rooms/v2/connectioninfo failed AppId=\"%s\" RoomId=\"%s\" ResponseCode=%d Response=\"%s\""),
				*AppId, *RoomId, Response->GetResponseCode(), *Response->GetContentAsString());
			OnComplete.ExecuteIfBound(false, FString(), 0);
			return;
		}

		TSharedPtr<FJsonObject> jsonResponseBody;
		TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Response->GetContentAsString());
		if (!FJsonSerializer::Deserialize(JsonReader, jsonResponseBody))
		{
			UE_LOG(LogHathoraSDK, Error, TEXT("rooms/v2/connectioninfo failed to deserialize json AppId=\"%s\" RoomId=\"%s\" Response=\"%s\""), 
				*AppId, *RoomId, *Response->GetContentAsString());
			OnComplete.ExecuteIfBound(false, FString(), 0);
			return;
		}

		FString status;
		if (!jsonResponseBody->TryGetStringField("status", status))
		{
			UE_LOG(LogHathoraSDK, Error, TEXT("rooms/v2/connectioninfo response missing status AppId=\"%s\" RoomId=\"%s\" Response=\"%s\""),
				*AppId, *RoomId, *Response->GetContentAsString());
			OnComplete.ExecuteIfBound(false, FString(), 0);
			return;
		}

		if (status == TEXT("starting")) {
			if ((FPlatformTime::Seconds() - StartTime) <= GetConnectionInfoTimeoutSeconds)
			{
				// Retry
				double delay = 1.0f;
				if (GetConnectionInfoDelaysSeconds.Num() > 0)
				{
					delay = FCString::Atof(*GetConnectionInfoDelaysSeconds[FGenericPlatformMath::Min(RetryCount, GetConnectionInfoDelaysSeconds.Num() - 1)]);
				}

				auto retry = [this, AppId, RoomId, StartTime, RetryCount, OnComplete]() {
					GetConnectionInfo_Internal(AppId, RoomId, StartTime, RetryCount + 1, OnComplete);
				};

				FTimerHandle unusedHandle;
				GWorld->GetTimerManager().SetTimer(unusedHandle, retry, delay, false);
				return;
			}
			else
			{
				// Timeout
				UE_LOG(LogHathoraSDK, Warning, TEXT("rooms/v2/connectioninfo timed out waiting for room to be active AppId=\"%s\" RoomId=\"%s\""),
					*AppId, *RoomId, *Response->GetContentAsString());
				OnComplete.ExecuteIfBound(false, FString(), 0);
				return;
			}
		}
		else if (status != TEXT("active")) {
			UE_LOG(LogHathoraSDK, Error, TEXT("rooms/v2/connectioninfo response has invalid status AppId=\"%s\" RoomId=\"%s\" Response=\"%s\""),
				*AppId, *RoomId, *Response->GetContentAsString());
			OnComplete.ExecuteIfBound(false, FString(), 0);
			return;
		}

		const TSharedPtr<FJsonObject>* jsonExposedPort;
		if (!jsonResponseBody->TryGetObjectField("exposedPort", jsonExposedPort))
		{
			UE_LOG(LogHathoraSDK, Error, TEXT("rooms/v2/connectioninfo response missing exposedPort AppId=\"%s\" RoomId=\"%s\" Response=\"%s\""),
				*AppId, *RoomId, *Response->GetContentAsString());
			OnComplete.ExecuteIfBound(false, FString(), 0);
			return;
		}

		FString host;
		if (!jsonExposedPort->Get()->TryGetStringField("host", host))
		{
			UE_LOG(LogHathoraSDK, Error, TEXT("rooms/v2/connectioninfo response missing host AppId=\"%s\" RoomId=\"%s\" Response=\"%s\""),
				*AppId, *RoomId, *Response->GetContentAsString());
			OnComplete.ExecuteIfBound(false, FString(), 0);
			return;
		}

		int32 port = 0;
		if (!jsonExposedPort->Get()->TryGetNumberField("port", port))
		{
			UE_LOG(LogHathoraSDK, Error, TEXT("rooms/v2/connectioninfo response missing port AppId=\"%s\" RoomId=\"%s\" Response=\"%s\""),
				*AppId, *RoomId, *Response->GetContentAsString());
			OnComplete.ExecuteIfBound(false, FString(), 0);
			return;
		}

		// Success
		OnComplete.ExecuteIfBound(true, host, port);
		});

	request->ProcessRequest();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FHathoraSDKModule, HathoraSDK)