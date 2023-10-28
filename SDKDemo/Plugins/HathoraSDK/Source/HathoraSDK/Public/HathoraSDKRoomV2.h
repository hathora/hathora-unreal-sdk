// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
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
struct FHathoraCreateRoomResult
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

UCLASS()
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
};
