// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HathoraTypes.h"
#include "HathoraGetRegionalPings.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FHathoraGetRegionalPingsComplete, FHathoraRegionPings, Result
);

UCLASS()
class HATHORASDK_API UHathoraGetRegionalPings : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

	// Get ping times to all available Hathora Cloud regions.
	// Each region is pinged NumPingsPerRegion times and the
	// minimum is returned.
	// Pings are returned in milliseconds.
	// @param OnComplete The delegate to call when the request is complete with averaged ping times.
	// @param NumPingsPerRegion The number of pings to send to each region.
	UFUNCTION(
		BlueprintCallable,
		meta =
			(BlueprintInternalUseOnly = "true",
			 Category = "HathoraSDK",
			 WorldContext = "WorldContextObject")
	)
	static UHathoraGetRegionalPings *GetRegionalPings(
		UObject *WorldContextObject,
		int32 NumPingsPerRegion = 3
	);

	UPROPERTY(BlueprintAssignable)
	FHathoraGetRegionalPingsComplete OnComplete;

	int32 NumPingsPerRegion;
};