// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Misc/DateTime.h"
#include "HathoraSDKAPI.h"
#include "HathoraTypes.h"
#include "HathoraSDKRoomV2.generated.h"

USTRUCT(BlueprintType)
struct FHathoraExposedPort
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString TransportType;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	int32 Port = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString Host;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString Name;
};

USTRUCT(BlueprintType)
struct FHathoraAllocation
{
	GENERATED_BODY()

	// If the API returns null for unscheduledAt, this will be false.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	bool bUnscheduled = false;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FDateTime UnscheduledAt;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FDateTime ScheduledAt;

	// System generated unique identifier to a runtime
	// instance of your game server.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString ProcessId;

	// System generated unique identifier to an allocated
	// instance of a room.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString RoomAllocationId;
};

USTRUCT(BlueprintType)
struct FHathoraConnectionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString Status;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString RoomId;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FHathoraExposedPort ExposedPort;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	TArray<FHathoraExposedPort> AdditionalExposedPorts;
};

USTRUCT(BlueprintType)
struct FHathoraRoomConnectionInfoResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	int32 StatusCode = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString ErrorMessage;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FHathoraConnectionInfo Data;
};

UENUM(BlueprintType)
enum class EHathoraRoomStatus : uint8
{
	// A process is not allocated yet and the room is
	// waiting to be scheduled.
	Scheduling,

	// Ready to accept connections.
	Active,

	// Room is unallocated from the process but can be
	// rescheduled later with the same `roomId`.
	Suspended,

	// All associated metadata is deleted.
	Destroyed,

	Unknown UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FHathoraGetRoomInfoData
{
	GENERATED_BODY()

	// Metadata on an allocated instance of a room.
	// Use Status to determine if this value is valid.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FHathoraAllocation CurrentAllocation;

	// The allocation status of a room.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	EHathoraRoomStatus Status = EHathoraRoomStatus::Unknown;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	TArray<FHathoraAllocation> Allocations;

	// Optional configuration parameters for the room.
	// Can be any string including stringified JSON.
	// String is empty if null or not set.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString RoomConfig;

	// Unique identifier to a game session or match.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString RoomId;

	// System generated unique identifier for an application.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString AppId;
};

USTRUCT(BlueprintType)
struct FHathoraGetRoomInfoResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	int32 StatusCode = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString ErrorMessage;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FHathoraGetRoomInfoData Data;
};

USTRUCT(BlueprintType)
struct FHathoraProcessRoomInfo
{
	GENERATED_BODY()

	// Metadata on an allocated instance of a room.
	// Use Status to determine if this value is valid.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FHathoraAllocation CurrentAllocation;

	// The allocation status of a room.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	EHathoraRoomStatus Status = EHathoraRoomStatus::Unknown;

	// Optional configuration parameters for the room.
	// Can be any string including stringified JSON.
	// String is empty if null or not set.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString RoomConfig;

	// Unique identifier to a game session or match.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString RoomId;

	// System generated unique identifier for an application.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString AppId;
};

USTRUCT(BlueprintType)
struct FHathoraGetRoomsForProcessResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	int32 StatusCode = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString ErrorMessage;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	TArray<FHathoraProcessRoomInfo> Data;
};

USTRUCT(BlueprintType)
struct FHathoraDestroyRoomResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	int32 StatusCode = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString ErrorMessage;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	bool bDestroyed = false;
};

USTRUCT(BlueprintType)
struct FHathoraSuspendRoomResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	int32 StatusCode = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString ErrorMessage;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	bool bSuspended = false;
};

USTRUCT(BlueprintType)
struct FHathoraUpdateRoomConfigResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	int32 StatusCode = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString ErrorMessage;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	bool bUpdated = false;
};

UCLASS(BlueprintType)
class HATHORASDK_API UHathoraSDKRoomV2 : public UHathoraSDKAPI
{
	GENERATED_BODY()

public:
	UDELEGATE()
	DECLARE_DYNAMIC_DELEGATE_OneParam(FHathoraOnRoomConnectionInfo, FHathoraRoomConnectionInfoResult, Result);

	// Create a new room for an existing application. Poll the GetConnectionInfo()
	// endpoint to get connection details for an active room.
	// @param Region The region to create the room in.
	// @param RoomConfig Optional configuration parameters for the room. Can be
	//                    any string including stringified JSON. It is accessible
	//                    from the room via GetRoomInfo().
	// @param RoomId Unique identifier to a game session or match. Leave empty to
	//                use the default system generated ID.
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | RoomV2")
	void CreateRoom(EHathoraCloudRegion Region, FString RoomConfig, FString RoomId, FHathoraOnRoomConnectionInfo OnComplete);

	UDELEGATE()
	DECLARE_DYNAMIC_DELEGATE_OneParam(FHathoraOnGetRoomInfo, FHathoraGetRoomInfoResult, Result);

	// Retrieve current and historical allocation data for a room.
	// @param RoomId Unique identifier to a game session or match.
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | RoomV2")
	void GetRoomInfo(FString RoomId, FHathoraOnGetRoomInfo OnComplete);

	UDELEGATE()
	DECLARE_DYNAMIC_DELEGATE_OneParam(FHathoraOnGetRoomsForProcess, FHathoraGetRoomsForProcessResult, Result);

	// Get all active rooms for a given process.
	// @param ProcessId System generated unique identifier to a runtime instance
	//                   of your game server.
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | RoomV2")
	void GetActiveRoomsForProcess(FString ProcessId, FHathoraOnGetRoomsForProcess OnComplete);

	// Get all inactive rooms for a given process.
	// @param ProcessId System generated unique identifier to a runtime instance
	//                   of your game server.
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | RoomV2")
	void GetInactiveRoomsForProcess(FString ProcessId, FHathoraOnGetRoomsForProcess OnComplete);

	UDELEGATE()
	DECLARE_DYNAMIC_DELEGATE_OneParam(FHathoraOnDestroyRoom, FHathoraDestroyRoomResult, Result);

	// Destroy a room. All associated metadata is deleted.
	// @param RoomId Unique identifier to a game session or match.
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | RoomV2")
	void DestroyRoom(FString RoomId, FHathoraOnDestroyRoom OnComplete);

	UDELEGATE()
	DECLARE_DYNAMIC_DELEGATE_OneParam(FHathoraOnSuspendRoom, FHathoraSuspendRoomResult, Result);

	// Suspend a room. The room is unallocated from the process but
	// can be rescheduled later using the same roomId.
	// @param RoomId Unique identifier to a game session or match.
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | RoomV2")
	void SuspendRoom(FString RoomId, FHathoraOnSuspendRoom OnComplete);

	// Poll this endpoint to get connection details to a room.
	// Clients can call this endpoint without authentication.
	// @param RoomId Unique identifier to a game session or match.
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | RoomV2")
	void GetConnectionInfo(FString RoomId, FHathoraOnRoomConnectionInfo OnComplete);

	UDELEGATE()
	DECLARE_DYNAMIC_DELEGATE_OneParam(FHathoraOnUpdateRoomConfig, FHathoraUpdateRoomConfigResult, Result);

	// Update the roomConfig variable for a room.
	// @param RoomId Unique identifier to a game session or match.
	// @param RoomConfig Optional configuration parameters for the room. Can be
	//                    any string including stringified JSON. It is accessible
	//                    from the room via GetRoomInfo().
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | RoomV2")
	void UpdateRoomConfig(FString RoomId, FString RoomConfig, FHathoraOnUpdateRoomConfig OnComplete);

private:
	static FHathoraAllocation ParseAllocation(const TSharedPtr<FJsonObject>& AllocationJson);
	static EHathoraRoomStatus ParseRoomStatus(const FString& StatusString);

	void GetRoomsForProcess(FString ProcessId, bool bActive, FHathoraOnGetRoomsForProcess OnComplete);
};
