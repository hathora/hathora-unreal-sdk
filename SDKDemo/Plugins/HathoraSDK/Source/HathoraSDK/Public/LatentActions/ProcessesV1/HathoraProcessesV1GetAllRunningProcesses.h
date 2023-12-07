// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HathoraTypes.h"
#include "HathoraProcessesV1LatentCommon.h"
#include "HathoraProcessesV1GetAllRunningProcesses.generated.h"

UCLASS()
class HATHORASDK_API UHathoraProcessesV1GetAllRunningProcesses : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

	// Retrieve 10 most recently started process objects for an application.
	UFUNCTION(
		BlueprintCallable,
		meta =
			(BlueprintInternalUseOnly = "true",
			 Category = "HathoraSDK | ProcessesV1",
			 WorldContext = "WorldContextObject")
	)
	static UHathoraProcessesV1GetAllRunningProcesses *GetAllRunningProcesses(
		UHathoraSDKProcessesV1 *HathoraSDKProcessesV1,
		UObject *WorldContextObject
	);

	UPROPERTY(BlueprintAssignable)
	FHathoraProcessesV1OnProcessInfos OnComplete;

	UPROPERTY()
	UHathoraSDKProcessesV1 *HathoraSDKProcessesV1;
};