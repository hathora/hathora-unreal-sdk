// Copyright 2023 Hathora, Inc.
#pragma once

#include "CoreMinimal.h"
#include "HathoraTypes.h"
#include "HathoraSDKConfig.h"
#include "Delegates/Delegate.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HathoraSDK.generated.h"

class UHathoraSDKAuthV1;
class UHathoraSDKDiscoveryV2;
class UHathoraSDKLobbyV3;
class UHathoraSDKProcessesV2;
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
	UE_DEPRECATED(4.0, "Use GetPingsForRegions instead.")
	static void GetRegionalPings(const FHathoraOnGetRegionalPings& OnComplete, int32 NumPingsPerRegion = 3);

	// Get ping times to the specified regions.
	// Each region is pinged NumPingsPerRegion times and the
	// minimum is returned.
	// Pings are returned in milliseconds.
	// @param RegionUrls A map of region names to region URL to ping (include `:port` in the URL string if using EHathoraPingType::UDPEcho).
	// @param PingType The type of ping/protocol to use.
	// @param OnComplete The delegate to call when the request is complete with averaged ping times.
	// @param NumPingsPerRegion The number of pings to send to each region.
	static void GetPingsForRegions(TMap<FString, FString> RegionUrls, EHathoraPingType PingType, const FHathoraOnGetRegionalPings& OnComplete, int32 NumPingsPerRegion = 3);

	// Region names (the keys in the map) are the same as the enum values in EHathoraCloudRegion.
	// NOTE: This function may uses hardcoded values for the region URLs and may
	// be outdated. It is recommended that your backend provides the region URLs
	// to use in GetPingsForRegions instead.
	UFUNCTION(BlueprintPure)
	static TMap<FString, FString> GetRegionMap();

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
	bool IsLoggedIn();

	UPROPERTY(BlueprintReadOnly, Category="HathoraSDK")
	UHathoraSDKAuthV1* AuthV1;

	UPROPERTY(BlueprintReadOnly, Category="HathoraSDK")
	UHathoraSDKDiscoveryV2* DiscoveryV2;

	UPROPERTY(BlueprintReadOnly, Category="HathoraSDK")
	UHathoraSDKLobbyV3* LobbyV3;

	UPROPERTY(BlueprintReadOnly, Category="HathoraSDK")
	UHathoraSDKProcessesV2* ProcessesV2;

	UPROPERTY(BlueprintReadOnly, Category="HathoraSDK")
	UHathoraSDKRoomV2* RoomV2;

private:
	void SetCredentials(FString AppId, FHathoraSDKSecurity Security);
};
