// Copyright 2023 Hathora, Inc. All Rights Reserved.

#include "HathoraSDKBPLibrary.h"

void UFHathoraSDKBPLibrary::GetPingServiceEndpoints(const FPingServiceEndpointsDynamicDelegate& OnComplete)
{
	FHathoraSDKModule* HathoraSDK = FHathoraSDKModule::Get();

	if (HathoraSDK == nullptr)
	{
		UE_LOG(LogHathoraSDK, Warning, TEXT("GetPingServiceEndpoints called with no active HathoraSDK module"));
		TArray<FDiscoveredPingEndpoint> emptyResult;
		OnComplete.ExecuteIfBound(emptyResult);
		return;
	}

	FHathoraSDKModule::FPingServiceEndpointsDelegate Delegate;
	Delegate.BindLambda([OnComplete](TArray<FDiscoveredPingEndpoint> OutPingEndpoints)
	{
		OnComplete.ExecuteIfBound(OutPingEndpoints);
	});
	HathoraSDK->GetPingServiceEndpoints(Delegate);
}

void UFHathoraSDKBPLibrary::GetAllRegionalPings(const FOnGetRegionalPingsDynamicDelegate& OnComplete)
{
	FHathoraSDKModule* HathoraSDK = FHathoraSDKModule::Get();

	if (HathoraSDK == nullptr)
	{
		UE_LOG(LogHathoraSDK, Warning, TEXT("GetRegionalPings called with no active HathoraSDK module"));
	FRegionalPings Result;
		OnComplete.ExecuteIfBound(Result);
		return;
	}

	FHathoraSDKModule::FOnGetRegionalPingsDelegate Delegate;
	Delegate.BindLambda([OnComplete](const TMap<FString, int32>& OutPingMap)
	{
		FRegionalPings Result;
		for (auto& Elem : OutPingMap)
		{
			Result.PingMap.Add(Elem.Key, Elem.Value);
		}
		OnComplete.ExecuteIfBound(Result);
	});
	HathoraSDK->GetRegionalPings(TArray<FDiscoveredPingEndpoint>(), Delegate);
}

void UFHathoraSDKBPLibrary::GetConnectionInfo(const FString& AppId, const FString& RoomId, const FConnectionInfoDynamicDelegate& OnComplete)
{
	FHathoraSDKModule* HathoraSDK = FHathoraSDKModule::Get();

	if (HathoraSDK == nullptr)
	{
		UE_LOG(LogHathoraSDK, Warning, TEXT("GetConnectionInfo called with no active HathoraSDK module"));
		OnComplete.ExecuteIfBound(false, FString(), 0);
		return;
	}

	FHathoraSDKModule::FConnectionInfoDelegate Delegate;
	Delegate.BindLambda([OnComplete](bool Success, FString Host, int32 Port)
	{
		OnComplete.ExecuteIfBound(Success, Host, Port);
	});
	HathoraSDK->GetConnectionInfo(AppId, RoomId, Delegate);
}
