// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "HathoraSDKAPI.h"
#include "HathoraTypes.h"
#include "Misc/DateTime.h"
#include "HathoraSDKLobbyV3.generated.h"

UENUM(BlueprintType)
enum class EHathoraLobbyVisibility : uint8
{
	// The player who created the room must share the roomId with their friends.
	Private,

	// Visible in the public lobby list, anyone can join.
	Public,

	// For testing with a server running locally.
	Local,

	Unknown UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FHathoraLobbyInfo
{
	GENERATED_BODY()

	// User-defined identifier for a lobby. Defaults to the RoomId
	// if a ShortCode was not specified in CreateLobby().
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString ShortCode;

	// When the lobby was created.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FDateTime CreatedAt;

	// Email address for the user that created the lobby.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString CreatedBy;

	// Optional configuration parameters for the room.
	// Can be any string including stringified JSON.
	// String is empty if null or not set.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString RoomConfig;

	// Types of lobbies a player can create.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	EHathoraLobbyVisibility Visibility = EHathoraLobbyVisibility::Unknown;

	// Which region the lobby is in.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	EHathoraCloudRegion Region = EHathoraCloudRegion::Unknown;

	// Unique identifier to a game session or match.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString RoomId;

	// System generated unique identifier for an application.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString AppId;
};

USTRUCT(BlueprintType)
struct FHathoraLobbyInfoResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	int32 StatusCode = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString ErrorMessage;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FHathoraLobbyInfo Data;
};

USTRUCT(BlueprintType)
struct FHathoraLobbyInfosResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	int32 StatusCode = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString ErrorMessage;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	TArray<FHathoraLobbyInfo> Data;
};

UCLASS(BlueprintType)
class HATHORASDK_API UHathoraSDKLobbyV3 : public UHathoraSDKAPI
{
	GENERATED_BODY()

public:
	UDELEGATE()
	DECLARE_DYNAMIC_DELEGATE_OneParam(FHathoraOnLobbyInfo, FHathoraLobbyInfoResult, Result);

	// Create a new lobby for an application. A lobby object is a wrapper around a room object.
	// With a lobby, you get additional functionality like configuring the visibility of the room,
	// managing the state of a match, and retrieving a list of public lobbies to display to players.
	// @param Visibility Types of lobbies a player can create.
	// @param RoomConfig Optional configuration parameters for the room. Can be
	//                   any string including stringified JSON. It is accessible
	//                   from the room via GetRoomInfo().
	// @param Region The region to create the room in.
	// @param ShortCode Optional user-defined identifier for a lobby. Leave empty
	//                  to reference the lobby using the RoomId only.
	// @param RoomId Unique identifier to a game session or match. Leave empty to
	//               use the default system generated ID.
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | LobbyV3")
	void CreateLobby(
		EHathoraLobbyVisibility Visibility,
		FString RoomConfig,
		EHathoraCloudRegion Region,
		FString ShortCode,
		FString RoomId,
		FHathoraOnLobbyInfo OnComplete
	);

	UDELEGATE()
	DECLARE_DYNAMIC_DELEGATE_OneParam(FHathoraOnLobbyInfos, FHathoraLobbyInfosResult, Result);

	// Get all public active lobbies for a given application. Use this endpoint to display
	// all public lobbies that a player can join in the game client. Use
	// ListRegionActivePublicLobbies() to only see lobbies in a specific region.
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | LobbyV3")
	void ListAllActivePublicLobbies(FHathoraOnLobbyInfos OnComplete);

	// Get all active lobbies for a given application, filtered by Region.
	// Use this endpoint to display all public lobbies that a player can join
	// in the game client. Use ListAllActivePublicLobbies() to see all lobbies.
	// @param Region Filter the returned lobbies by the provided region.
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | LobbyV3")
	void ListRegionActivePublicLobbies(EHathoraCloudRegion Region, FHathoraOnLobbyInfos OnComplete);

	// Get details for a lobby.
	// @param RoomId Unique identifier to a game session or match.
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | LobbyV3")
	void GetLobbyInfoByRoomId(FString RoomId, FHathoraOnLobbyInfo OnComplete);

	// Get details for a lobby. If 2 or more lobbies have the same shortCode, then the most
	// recently created lobby will be returned.
	// @param ShortCode User-defined identifier for a lobby.
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | LobbyV3")
	void GetLobbyInfoByShortCode(FString ShortCode, FHathoraOnLobbyInfo OnComplete);

private:
	static FString GetVisibilityString(EHathoraLobbyVisibility Visibility);
	static EHathoraLobbyVisibility ParseVisibility(const FString& VisibilityString);

	void ListActivePublicLobbies(TArray<TPair<FString, FString>> QueryOptions, FHathoraOnLobbyInfos OnComplete);

	static FHathoraLobbyInfo ParseLobbyInfo(TSharedPtr<FJsonObject> LobbyInfoJson);
};
