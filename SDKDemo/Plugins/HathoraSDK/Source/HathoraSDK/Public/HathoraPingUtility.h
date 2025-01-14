// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "HathoraTypes.h"
#include "HathoraPingUtility.generated.h"

UCLASS()
class HATHORASDK_API UHathoraPingUtility : public UObject
{
	GENERATED_BODY()

public:

	void GetPingsForRegions(TMap<FString, FString> InRegionUrls, EHathoraPingType InPingType, const FHathoraOnGetRegionalPings& OnComplete, int32 InNumPingsPerRegion = 3);

private:
	TMap<FString, FString> RegionUrls;
	EHathoraPingType PingType;

	FHathoraOnGetRegionalPings OnGetRegionalPingsComplete;
	TArray<FHathoraDiscoveredPingEndpoint> PingEndpoints;
	int32 NumPingsPerRegion;
	int32 NumPingsPerRegionCompleted;
	TSharedPtr<TMap<FString, TArray<int32>>> PingResults;

	void PingEachRegion();
};
