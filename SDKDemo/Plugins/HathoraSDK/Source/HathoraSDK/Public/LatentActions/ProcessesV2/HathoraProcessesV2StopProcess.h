// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HathoraTypes.h"
#include "HathoraProcessesV2LatentCommon.h"
#include "HathoraProcessesV2StopProcess.generated.h"

UCLASS()
class HATHORASDK_API UHathoraProcessesV2StopProcess : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

	// Stops a process immediately.
	// @param ProcessId System generated unique identifier to a runtime
	//                  instance of your game server.
	UFUNCTION(
		BlueprintCallable,
		meta =
			(BlueprintInternalUseOnly = "true",
			 Category = "HathoraSDK | ProcessesV2",
			 WorldContext = "WorldContextObject")
	)
	static UHathoraProcessesV2StopProcess *StopProcess(
		UHathoraSDKProcessesV2 *HathoraSDKProcessesV2,
		UObject *WorldContextObject,
		FString ProcessId
	);

	UPROPERTY(BlueprintAssignable)
	FHathoraProcessesV2OnStopProcess OnComplete;

	UPROPERTY()
	UHathoraSDKProcessesV2 *HathoraSDKProcessesV2;

	FString ProcessId;
};