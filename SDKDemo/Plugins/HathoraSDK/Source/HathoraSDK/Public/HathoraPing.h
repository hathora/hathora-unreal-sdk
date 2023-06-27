// Copyright 2023 Hathora, Inc.
#pragma once

#include "CoreMinimal.h"
#include "HathoraSdkConfig.h"
#include "Delegates/Delegate.h"
#include "HathoraSDK/Private/DiscoveredPingEndpoint.h"
#include "HathoraPing.generated.h"

UCLASS()
class HATHORASDK_API UHathoraPing : public UObject
{
	GENERATED_BODY()
public:
	UHathoraPing(const FObjectInitializer& ObjectInitializer);
	typedef TDelegate<void(const TMap<FString, int32>&)> FOnGetRegionalPingsDelegate;

	void SetConfig(UHathoraSdkConfig* NewConfig) { HathoraSdkConfig = NewConfig; }

	// Get ping times to all available Hathora Cloud regions
	// pings are returned in milliseconds
	void GetRegionalPings(const FOnGetRegionalPingsDelegate& OnComplete);

private:
	UPROPERTY()
	UHathoraSdkConfig* HathoraSdkConfig;
	DECLARE_DELEGATE_TwoParams(FOnGetPingDelegate, int32 /* Ping */, bool /* bWasSuccessful */);
	static void PingUrlsAndAggregateTimes(const TArray<FDiscoveredPingEndpoint>& PingEndpoints, int32 MeasurementsToTake, const FOnGetRegionalPingsDelegate& OnComplete);
	static void GetPingTime(const FDiscoveredPingEndpoint& PingEndpoint, int32 PingID, const FOnGetPingDelegate& OnComplete);
	static void GetMedianPingPerRegion(TMap<FString,TArray<int32>>& AllPingMeasurementsByRegion, const FOnGetRegionalPingsDelegate& OnComplete);
};
