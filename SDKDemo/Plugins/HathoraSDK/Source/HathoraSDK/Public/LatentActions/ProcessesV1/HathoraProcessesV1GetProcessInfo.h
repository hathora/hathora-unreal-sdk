// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HathoraTypes.h"
#include "HathoraProcessesV1LatentCommon.h"
#include "HathoraProcessesV1GetProcessInfo.generated.h"

UCLASS()
class HATHORASDK_API UHathoraProcessesV1GetProcessInfo : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

	// Get details for a process.
	// @param ProcessId System generated unique identifier to a runtime
	//                  instance of your game server.
	UFUNCTION(
		BlueprintCallable,
		meta =
			(BlueprintInternalUseOnly = "true",
			 Category = "HathoraSDK | ProcessesV1",
			 WorldContext = "WorldContextObject")
	)
	static UHathoraProcessesV1GetProcessInfo *GetProcessInfo(
		UHathoraSDKProcessesV1 *HathoraSDKProcessesV1,
		UObject *WorldContextObject,
		FString ProcessId
	);

	UPROPERTY(BlueprintAssignable)
	FHathoraProcessesV1OnProcessInfo OnComplete;

	UPROPERTY()
	UHathoraSDKProcessesV1 *HathoraSDKProcessesV1;

	FString ProcessId;
};