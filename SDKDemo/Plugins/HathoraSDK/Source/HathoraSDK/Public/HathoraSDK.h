// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "HttpRetrySystem.h"

#include "HathoraSDKTypes.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHathoraSDK, Log, All)

class HATHORASDK_API FHathoraSDKModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static FHathoraSDKModule* Get()
	{
		static FName hathoraSdk(TEXT("HathoraSDK"));
		return &FModuleManager::LoadModuleChecked< FHathoraSDKModule >(hathoraSdk);
	}

	/// <summary>
	/// Provide an implementation of the Http Retry System Manager so underlying rest calls can be retried
	/// on errors that are possibly temporary
	/// </summary>
	void SetRetryManager(TSharedPtr<class FHttpRetrySystem::FManager>& httpRetryManager);

	/// <summary>
	/// Asynchronous call to get a map of current Hathora regions to client pings. An empty array for
	/// PingEndpoints will call GetPingServiceEndpoints and ping all regions.
	/// </summary>
	typedef TDelegate<void(const TMap<FString, int32>& /* PingMap */)> FOnGetRegionalPingsDelegate;
	void GetRegionalPings(TArray<FDiscoveredPingEndpoint> PingEndpoints, const FOnGetRegionalPingsDelegate& OnComplete);

	/// <summary>
	/// Asynchronous call to get a list of all the Hathora regions and their ping endpoints
	/// </summary>
	DECLARE_DELEGATE_OneParam(FPingServiceEndpointsDelegate, TArray<FDiscoveredPingEndpoint> /* PingEndpoints */);
	void GetPingServiceEndpoints(const FPingServiceEndpointsDelegate& OnComplete);

	/// <summary>
	/// Get the hostname + port given an AppId and RoomId
	/// This allows matchmaking services to return a result while the room is starting and not yet active
	/// </summary>
	DECLARE_DELEGATE_ThreeParams(FConnectionInfoDelegate, bool /* Success */, FString /* Host */, int32 /* Port */);
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK")
	void GetConnectionInfo(const FString& AppId, const FString& RoomId, const FConnectionInfoDelegate& OnComplete);

protected:

	//
	// Configuration stored in Game.ini under [HathoraSDK]
	//

	/// <summary>
	/// Base URL to use for Hathora API calls (https://api.hathora.dev)
	/// </summary>
	FString BaseUrl;

	/// <summary>
	/// Total timeout to get the connection info for a room (that may have been starting)
	/// </summary>
	float GetConnectionInfoTimeoutSeconds;

	/// <summary>
	/// Escalating sequence of retry delays for a room that's still starting
	/// The last value is re-used for a long startup
	/// </summary>
	TArray<FString> GetConnectionInfoDelaysSeconds;

protected:

	void GetRegionalPings_Internal(TArray<FDiscoveredPingEndpoint> PingEndpoints, const FOnGetRegionalPingsDelegate& OnComplete);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> NewRequest();
	void GetConnectionInfo_Internal(const FString& AppId, const FString& RoomId, double StartTime, int32 RetryCount, const FConnectionInfoDelegate& OnComplete);

	TSharedPtr<class FHttpRetrySystem::FManager> HttpRetryManager;
};
