// Copyright 2023 Hathora, Inc.
#pragma once

#include "CoreMinimal.h"
#include "HathoraTypes.h"
#include "HathoraSDKConfig.h"
#include "Delegates/Delegate.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HathoraSDK.generated.h"

class UHathoraSDKAppV1;
class UHathoraSDKAuthV1;
class UHathoraSDKDiscoveryV1;
class UHathoraSDKLobbyV3;
class UHathoraSDKProcessesV1;
class UHathoraSDKRoomV2;

UCLASS(BlueprintType)
class HATHORASDK_API UHathoraSDK : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Get ping times to all available Hathora Cloud regions.
	// Each region is pinged NumPingsPerRegion times and the
	// minimum is returned.
	// Pings are returned in milliseconds.
	// @param OnComplete The delegate to call when the request is complete with averaged ping times.
	// @param NumPingsPerRegion The number of pings to send to each region.
	static void GetRegionalPings(const FHathoraOnGetRegionalPings& OnComplete, int32 NumPingsPerRegion = 3);

	// Create an instance of the Hathora SDK using the AppId, and DevToken if specified,
	// from Game.ini. See Project Settings > Plugins > HathoraSDK for more information.
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK")
	static UHathoraSDK* CreateHathoraSDK();

	// Get the `HATHORA_*` environment variables that are injected into the game server.
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK")
	static FHathoraServerEnvironment GetServerEnvironment();

	// Get the string representation of the region enum value.
	UFUNCTION(BlueprintPure, Category = "HathoraSDK")
	static FString GetRegionString(EHathoraCloudRegion Region);

	// Parse the string representation to get the enum value.
	UFUNCTION(BlueprintPure, Category = "HathoraSDK")
	static EHathoraCloudRegion ParseRegion(FString RegionString);

	UFUNCTION(BlueprintPure, Category = "HathoraSDK")
	static FString ParseErrorMessage(FString Content);

	// Set the auth token to use for all requests; primarily on the
	// client after the player has logged in.
	// @param Token The JWT auth token to use for all requests.
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK")
	void SetAuthToken(FString Token);

	UFUNCTION(BlueprintPure, Category = "HathoraSDK")
	FString GetAuthToken();

	UFUNCTION(BlueprintPure, Category = "HathoraSDK")
	bool IsLoggedIn();

	UPROPERTY(BlueprintReadOnly, Category="HathoraSDK")
	UHathoraSDKAppV1* AppV1;

	UPROPERTY(BlueprintReadOnly, Category="HathoraSDK")
	UHathoraSDKAuthV1* AuthV1;

	UPROPERTY(BlueprintReadOnly, Category="HathoraSDK")
	UHathoraSDKDiscoveryV1* DiscoveryV1;

	UPROPERTY(BlueprintReadOnly, Category="HathoraSDK")
	UHathoraSDKLobbyV3* LobbyV3;

	UPROPERTY(BlueprintReadOnly, Category="HathoraSDK")
	UHathoraSDKProcessesV1* ProcessesV1;

	UPROPERTY(BlueprintReadOnly, Category="HathoraSDK")
	UHathoraSDKRoomV2* RoomV2;

private:
	void SetCredentials(FString AppId, FHathoraSDKSecurity Security);
};
