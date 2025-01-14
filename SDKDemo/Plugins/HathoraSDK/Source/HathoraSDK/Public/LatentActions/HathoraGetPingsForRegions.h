// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HathoraTypes.h"
#include "HathoraGetPingsForRegions.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FHathoraGetPingsForRegionsComplete, FHathoraRegionPings, Result
);

UCLASS()
class HATHORASDK_API UHathoraGetPingsForRegions : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

	// Get ping times to the specified regions.
	// Each region is pinged NumPingsPerRegion times and the
	// minimum is returned.
	// Pings are returned in milliseconds.
	// @param RegionUrls A map of region names to region URL to ping (include `:port` in the URL string if using EHathoraPingType::UDPEcho).
	// @param PingType The type of ping/protocol to use.
	// @param OnComplete The delegate to call when the request is complete with averaged ping times.
	// @param NumPingsPerRegion The number of pings to send to each region.
	UFUNCTION(
		BlueprintCallable,
		meta =
			(BlueprintInternalUseOnly = "true",
			 Category = "HathoraSDK",
			 WorldContext = "WorldContextObject")
	)
	static UHathoraGetPingsForRegions *GetPingsForRegions(
		UObject *WorldContextObject,
		TMap<FString, FString> RegionUrls,
		EHathoraPingType PingType,
		int32 NumPingsPerRegion = 3
	);

	UPROPERTY(BlueprintAssignable)
	FHathoraGetPingsForRegionsComplete OnComplete;

	TMap<FString, FString> RegionUrls;
	EHathoraPingType PingType;
	int32 NumPingsPerRegion;
};