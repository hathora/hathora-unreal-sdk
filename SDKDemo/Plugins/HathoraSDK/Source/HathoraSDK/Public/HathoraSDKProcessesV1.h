// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "HathoraSDKAPI.h"
#include "HathoraTypes.h"
#include "HathoraSDKProcessesV1.generated.h"

USTRUCT(BlueprintType)
struct FHathoraProcessInfo
{
	GENERATED_BODY()

	// Measures network traffic leaving the process in bytes.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	int32 EgressedBytes;

	// Tracks the number of active connections to a process.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	int32 ActiveConnections;

	// The last time RoomsAllocated was updated.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FDateTime RoomsAllocatedUpdatedAt;

	// Tracks the number of rooms that have been allocated to the process.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	int32 RoomsAllocated;

	// Process in drain will not accept any new rooms.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	bool bDraining;

	// Whether or not the process has terminated.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	bool bTerminated;

	// When the process has been terminated. Invalid if bTerminated is false.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FDateTime TerminatedAt;

	// Whether or not the process ever transitioned to stopping.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	bool bStopping;

	// When the process is issued to stop. We use this to determine when
	// we should stop billing. Invalid if bStopping is false.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FDateTime StoppingAt;

	// Whether or not the process ever transitioned to starting.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	bool bStarted;

	// When the process bound to the specified port. We use this to
	// determine when we should start billing. Invalid if bStarted is false.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FDateTime StartedAt;

	// When the process started being provisioned. Always valid.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FDateTime StartingAt;

	// Governs how many rooms can be scheduled in a process.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	int32 RoomsPerProcess;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	TArray<FHathoraExposedPort> AdditionalExposedPorts;

	// Connection details for an active process.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FHathoraExposedPort ExposedPort;

	// The region that the process is running in.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	EHathoraCloudRegion Region;

	// System generated unique identifier to a runtime instance of your game server.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString ProcessId;

	// System generated id for a deployment. Increments by 1.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	int32 DeploymentId;

	// System generated unique identifier for an application.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString AppId;
};

USTRUCT(BlueprintType)
struct FHathoraProcessInfoResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	int32 StatusCode = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString ErrorMessage;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FHathoraProcessInfo Data;
};

USTRUCT(BlueprintType)
struct FHathoraProcessInfosResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	int32 StatusCode = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString ErrorMessage;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	TArray<FHathoraProcessInfo> Data;
};

UCLASS(BlueprintType)
class HATHORASDK_API UHathoraSDKProcessesV1 : public UHathoraSDKAPI
{
	GENERATED_BODY()

public:
	UDELEGATE()
	DECLARE_DYNAMIC_DELEGATE_OneParam(FHathoraOnProcessInfo, FHathoraProcessInfoResult, Result);

	UDELEGATE()
	DECLARE_DYNAMIC_DELEGATE_OneParam(FHathoraOnProcessInfos, FHathoraProcessInfosResult, Result);

	// Retrieve 10 most recently started process objects for an application.
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | ProcessV1")
	void GetAllRunningProcesses(FHathoraOnProcessInfos OnComplete);

	// Retrieve 10 most recently started process objects for an application,
	// filtered by Region.
	// @param Region Filter the returned processes by the provided region.
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | ProcessV1")
	void GetRegionRunningProcesses(EHathoraCloudRegion Region, FHathoraOnProcessInfos OnComplete);

	// Retrieve 10 most recently stopped process objects for an application.
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | ProcessV1")
	void GetAllStoppedProcesses(FHathoraOnProcessInfos OnComplete);

	// Retrieve 10 most recently stopped process objects for an application.
	// filtered by Region.
	// @param Region Filter the returned processes by the provided region.
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | ProcessV1")
	void GetRegionStoppedProcesses(EHathoraCloudRegion Region, FHathoraOnProcessInfos OnComplete);

	// Get details for a process.
	// @param ProcessId System generated unique identifier to a runtime
	//                  instance of your game server.
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | ProcessV1")
	void GetProcessInfo(FString ProcessId, FHathoraOnProcessInfo OnComplete);

private:
	static FHathoraProcessInfo ParseProcessInfo(TSharedPtr<FJsonObject> ProcessInfoJson);

	void GetProcesses(bool bRunning, TArray<TPair<FString, FString>> QueryOptions, FHathoraOnProcessInfos OnComplete);
};
