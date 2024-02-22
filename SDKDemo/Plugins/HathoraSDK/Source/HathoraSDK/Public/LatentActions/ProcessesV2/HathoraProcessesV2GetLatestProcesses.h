// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HathoraTypes.h"
#include "HathoraProcessesV2LatentCommon.h"
#include "HathoraProcessesV2GetLatestProcesses.generated.h"

UCLASS()
class HATHORASDK_API UHathoraProcessesV2GetLatestProcesses : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

	// Retrieve 10 most recently started process objects for an application.
	UFUNCTION(
		BlueprintCallable,
		meta =
			(BlueprintInternalUseOnly = "true",
			 Category = "HathoraSDK | ProcessesV2",
			 WorldContext = "WorldContextObject")
	)
	static UHathoraProcessesV2GetLatestProcesses *GetLatestProcesses(
		UHathoraSDKProcessesV2 *HathoraSDKProcessesV2,
		UObject *WorldContextObject,
		TArray<EHathoraProcessStatus> StatusFilter,
		TArray<EHathoraCloudRegion> RegionFilter
	);

	UPROPERTY(BlueprintAssignable)
	FHathoraProcessesV2OnProcessInfos OnComplete;

	UPROPERTY()
	UHathoraSDKProcessesV2 *HathoraSDKProcessesV2;

	UPROPERTY()
	TArray<EHathoraProcessStatus> StatusFilter;

	UPROPERTY()
	TArray<EHathoraCloudRegion> RegionFilter;
};