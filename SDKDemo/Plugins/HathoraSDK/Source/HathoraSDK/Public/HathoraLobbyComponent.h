// Copyright 2023 Hathora, Inc.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HathoraSDKLobbyV3.h"
#include "HathoraSDKRoomV2.h"
#include "HathoraLobbyComponent.generated.h"

class UHathoraSDK;

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class HATHORASDK_API UHathoraLobbyComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	//~ Begin UActorComponent Interface.
	virtual void BeginPlay() override;
	//~ End UActorComponent Interface.

	UFUNCTION(BlueprintPure, Category = "HathoraSDK | Lobby Component")
	UHathoraSDK* GetSDK() { return SDK; }

	UFUNCTION(BlueprintPure, Category = "HathoraSDK | Lobby Component")
	bool IsLoggedIn();

	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | Lobby Component")
	void CreateAndJoinLobby(
		EHathoraLobbyVisibility Visibility,
		FString RoomConfig,
		EHathoraCloudRegion Region,
		int32 ShortCodeLength = 4
	);

	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | Lobby Component")
	void CreateAndJoinLobbyCustomShortCode(
		EHathoraLobbyVisibility Visibility,
		FString RoomConfig,
		EHathoraCloudRegion Region,
		FString ShortCode
	);

	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | Lobby Component")
	void FetchPublicLobbies();

	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | Lobby Component")
	void JoinLobbyWithShortCode(FString ShortCode);

	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | Lobby Component")
	void JoinLobbyWithConnectionInfo(FHathoraConnectionInfo ConnectionInfo);

	UDELEGATE()
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLobbyComponentError, FString, Error);
	UPROPERTY(BlueprintAssignable, Category = "HathoraSDK | Lobby Component")
	FLobbyComponentError OnError;

	UDELEGATE()
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
		FLobbyComponentLobbyReady,
		FHathoraConnectionInfo, ConnectionInfo,
		FString, RoomConfig,
		EHathoraCloudRegion, Region
	);
	UPROPERTY(BlueprintAssignable, Category = "HathoraSDK | Lobby Component")
	FLobbyComponentLobbyReady OnLobbyReady;

private:
	UPROPERTY()
	UHathoraSDK* SDK = nullptr;

	TMap<FString, FHathoraConnectionInfo> PreReadyLobbies;

	typedef TDelegate<void(const FHathoraConnectionInfo&)> FHathoraOnGetGetLobbyConnectionInfo;
	void GetLobbyConnectionInfo(FString RoomId, FHathoraOnGetGetLobbyConnectionInfo OnResult);
	void GetLobbyConnectionInfo(FString RoomId, FString PortName, FHathoraOnGetGetLobbyConnectionInfo OnResult);
};
