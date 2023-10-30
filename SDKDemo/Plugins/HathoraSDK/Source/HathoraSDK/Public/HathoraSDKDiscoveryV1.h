// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "HathoraSDKAPI.h"
#include "HathoraTypes.h"
#include "HathoraSDKDiscoveryV1.generated.h"

UCLASS()
class HATHORASDK_API UHathoraSDKDiscoveryV1 : public UHathoraSDKAPI
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | DiscoveryV1")
	void GetRegionalPings(const FHathoraOnGetRegionalPings& OnComplete);

	UDELEGATE()
	DECLARE_DYNAMIC_DELEGATE_OneParam(FHathoraOnGetPingServiceEndpoints, const TArray<FHathoraDiscoveredPingEndpoint>&, Endpoints);

	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | DiscoveryV1")
	void GetPingServiceEndpoints(const FHathoraOnGetPingServiceEndpoints& OnComplete);

private:
	FHathoraOnGetRegionalPings OnGetRegionalPingsComplete;

	DECLARE_DELEGATE_TwoParams(FOnGetPingDelegate, int32 /* Ping */, bool /* bWasSuccessful */);

	UFUNCTION()
	void PingUrlsAndAggregateTimes(const TArray<FHathoraDiscoveredPingEndpoint>& PingEndpoints);
};
