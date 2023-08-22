// Copyright 2023 Hathora, Inc. All Rights Reserved.

#include "HathoraSDK.h"

#include "DiscoveredPingEndpoint.h"

#include "Async/Async.h"
#include "HAL/RunnableThread.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#endif

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

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FHathoraSDKModule, HathoraSDK)