// Copyright 2023 Hathora, Inc. All Rights Reserved.

#include "HathoraSDK.h"

#include "HathoraSDKTypes.h"

#include "Async/Async.h"
#include "HAL/RunnableThread.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "TimerManager.h"
#include "JsonObjectConverter.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#endif

#if PLATFORM_WINDOWS
#include "Windows/HideWindowsPlatformTypes.h"
#endif

#include <string>

#define LOCTEXT_NAMESPACE "FHathoraSDKModule"

int CollectRegionPing(const FDiscoveredPingEndpoint& Endpoint);

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

void FHathoraSDKModule::SetRetryManager(TSharedPtr<class FHttpRetrySystem::FManager>& httpRetryManager)
{
	HttpRetryManager = httpRetryManager;
}

void FHathoraSDKModule::GetPingServiceEndpoints(const FPingServiceEndpointsDelegate& OnComplete)
{
	TSharedRef< IHttpRequest, ESPMode::ThreadSafe > request = NewRequest();
	request->SetURL(FString::Format(TEXT("{0}/discovery/v1/ping"), { BaseUrl }));
	request->OnProcessRequestComplete().BindLambda([OnComplete](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) {
		TArray<FDiscoveredPingEndpoint> endpoints;
		if (bSuccess && Response.IsValid() && FJsonObjectConverter::JsonArrayStringToUStruct(Response->GetContentAsString(), &endpoints))
		{
			if (endpoints.Num() > 0)
			{
				UE_LOG(LogHathoraSDK, Warning, TEXT("/discovery/v1/ping returned an empty list of regions"));
			}

			OnComplete.ExecuteIfBound(endpoints);
		}
		else
		{
			UE_LOG(LogHathoraSDK, Warning, TEXT("Could not retrieve ping endpoints"))
			TArray<FDiscoveredPingEndpoint> emptyResult;
			OnComplete.ExecuteIfBound(emptyResult);
		}
	});
	request->ProcessRequest();
}

void FHathoraSDKModule::GetRegionalPings(TArray<FDiscoveredPingEndpoint> PingEndpoints, const FOnGetRegionalPingsDelegate& OnComplete)
{
	if (PingEndpoints.Num() == 0)
	{
		// Call GetPingServiceEndpoints to get the list of endpoints
		FPingServiceEndpointsDelegate Delegate;
		Delegate.BindLambda([this, OnComplete](TArray<FDiscoveredPingEndpoint> OutPingEndpoints)
		{
			GetRegionalPings_Internal(OutPingEndpoints, OnComplete);
		});
		GetPingServiceEndpoints(Delegate);
		return;
	}

	GetRegionalPings_Internal(PingEndpoints, OnComplete);
}

void FHathoraSDKModule::GetRegionalPings_Internal(TArray<FDiscoveredPingEndpoint> PingEndpoints, const FOnGetRegionalPingsDelegate& OnComplete)
{
	if (PingEndpoints.Num() == 0)
	{
		UE_LOG(LogHathoraSDK, Warning, TEXT("GetRegionalPings_Internal called with an empty list of endpoints"));
		TMap<FString, int32> emptyResult;
		OnComplete.ExecuteIfBound(emptyResult);
		return;
	}

	TSharedPtr<TMap<FString, int32>> pings = MakeShared<TMap<FString, int32>>();
	TSharedPtr<int32>                pendingPings = MakeShared<int32>(PingEndpoints.Num());

	if (PingEndpoints.Num() > 0)
	{
		for (const FDiscoveredPingEndpoint& endpoint : PingEndpoints)
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
}

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