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
	int32 Port;

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
	bool bUnscheduled;

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
struct FHathoraCreateRoomData
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
struct FHathoraCreateRoomResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	int32 StatusCode;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString ErrorMessage;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FHathoraCreateRoomData Data;
};

UENUM(BlueprintType)
enum class EHathoraRoomStatus : uint8
{
	// Could not parse the API response.
	Unknown,

	// A process is not allocated yet and the room is
	// waiting to be scheduled.
	Scheduling,

	// Ready to accept connections.
	Active,

	// Room is unallocated from the process but can be
	// rescheduled later with the same `roomId`.
	Suspended,

	// All associated metadata is deleted.
	Destroyed
};

USTRUCT(BlueprintType)
struct FHathoraGetRoomInfoData
{
	GENERATED_BODY()

	// Metadata on an allocated instance of a room.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FHathoraAllocation CurrentAllocation;

	// The allocation status of a room.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	EHathoraRoomStatus Status;

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
	int32 StatusCode;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString ErrorMessage;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FHathoraGetRoomInfoData Data;
};

UCLASS(BlueprintType)
class HATHORASDK_API UHathoraSDKRoomV2 : public UHathoraSDKAPI
{
	GENERATED_BODY()

public:
	UDELEGATE()
	DECLARE_DYNAMIC_DELEGATE_OneParam(FHathoraOnCreateRoom, FHathoraCreateRoomResult, Result);

	// Create a new room for an existing application. Poll the GetConnectionInfo()
	// endpoint to get connection details for an active room.
	// @param Region: The region to create the room in.
	// @param RoomConfig: Optional configuration parameters for the room. Can be
	//                    any string including stringified JSON. It is accessible
	//                    from the room via GetRoomInfo().
	// @param RoomId: Unique identifier to a game session or match. Leave empty to
	//                use the default system generated ID.
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | RoomV2")
	void CreateRoom(EHathoraCloudRegion Region, FString RoomConfig, FString RoomId, FHathoraOnCreateRoom OnComplete);

	UDELEGATE()
	DECLARE_DYNAMIC_DELEGATE_OneParam(FHathoraOnGetRoomInfo, FHathoraGetRoomInfoResult, Result);

	// Retrieve current and historical allocation data for a room.
	// @param RoomId: Unique identifier to a game session or match.
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | RoomV2")
	void GetRoomInfo(FString RoomId, FHathoraOnGetRoomInfo OnComplete);

private:
	static FHathoraAllocation ParseAllocation(const TSharedPtr<FJsonObject>& AllocationJson);
	static EHathoraRoomStatus ParseRoomStatus(const FString& StatusString);
};
