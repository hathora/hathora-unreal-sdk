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
	static void PingUrlsAndAggregateTimes(const TArray<FDiscoveredPingEndpoint>& PingEndpoints, int32 MeasurementsToTake, const FOnGetRegionalPingsDelegate& OnComplete);
	typedef TDelegate<void(TArray<double>& /* Measurements */, bool /* bSuccessful */)> FOnGetPingMeasurementsDelegate;
	static void GetPingMeasurements(const FDiscoveredPingEndpoint& PingEndpoint, int32 MeasurementsToTake, const FOnGetPingMeasurementsDelegate& OnComplete);
	static void GetMedianPingPerRegion(TMap<FString,TArray<int32>>& AllPingMeasurementsByRegion, const FOnGetRegionalPingsDelegate& OnComplete);
};
