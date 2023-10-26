// Copyright 2023 Hathora, Inc.
#pragma once

#include "CoreMinimal.h"
#include "HathoraSDKConfig.h"
#include "Delegates/Delegate.h"
#include "HathoraSDK/Private/DiscoveredPingEndpoint.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HathoraSDK.generated.h"

USTRUCT(BlueprintType)
struct FHathoraRegionPings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, int32> Pings;
};

UCLASS()
class HATHORASDK_API UHathoraSDK : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UDELEGATE()
	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGetRegionalPingsDelegate, FHathoraRegionPings, Result);

	// Get ping times to all available Hathora Cloud regions
	// pings are returned in milliseconds
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK")
	static void GetRegionalPings(const FOnGetRegionalPingsDelegate& OnComplete);

private:
	DECLARE_DELEGATE_TwoParams(FOnGetPingDelegate, int32 /* Ping */, bool /* bWasSuccessful */);
	static void PingUrlsAndAggregateTimes(const TArray<FDiscoveredPingEndpoint>& PingEndpoints, const FOnGetRegionalPingsDelegate& OnComplete);
	static void GetPingTime(const FDiscoveredPingEndpoint& PingEndpoint, const FOnGetPingDelegate& OnComplete);
};
