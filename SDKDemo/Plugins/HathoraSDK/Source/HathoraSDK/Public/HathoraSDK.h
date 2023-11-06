// Copyright 2023 Hathora, Inc.
#pragma once

#include "CoreMinimal.h"
#include "HathoraTypes.h"
#include "HathoraSDKConfig.h"
#include "Delegates/Delegate.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HathoraSDK.generated.h"

class UHathoraSDKAuthV1;
class UHathoraSDKDiscoveryV1;
class UHathoraSDKLobbyV3;
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
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK")
	static void GetRegionalPings(const FHathoraOnGetRegionalPings& OnComplete, int32 NumPingsPerRegion = 3);

	// Create an instance of the Hathora SDK.
	// @param AppId The ID of your Hathora App (e.g. "app-88871e9f-ca62-413c-beab-da226f2afd71").
	// @param Security The security configuration for your Hathora App; primarily used to provide the Dev Token.
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK")
	static UHathoraSDK* CreateHathoraSDK(FString AppId, FHathoraSDKSecurity Security);

	// Set the credentials of this SDK instance.
	// @param AppId The ID of your Hathora App (e.g. "app-88871e9f-ca62-413c-beab-da226f2afd71").
	// @param Security The security configuration for your Hathora App; primarily used to provide the Dev Token.
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK")
	void SetCredentials(FString AppId, FHathoraSDKSecurity Security);

	UPROPERTY(BlueprintReadOnly, Category="HathoraSDK")
	UHathoraSDKAuthV1* AuthV1;

	UPROPERTY(BlueprintReadOnly, Category="HathoraSDK")
	UHathoraSDKDiscoveryV1* DiscoveryV1;

	UPROPERTY(BlueprintReadOnly, Category="HathoraSDK")
	UHathoraSDKLobbyV3* LobbyV3;

	UPROPERTY(BlueprintReadOnly, Category="HathoraSDK")
	UHathoraSDKRoomV2* RoomV2;

private:
	FHathoraOnGetRegionalPings OnGetRegionalPingsComplete;

	UFUNCTION()
	void OnGetRegionalPingsCompleteWrapper(FHathoraRegionPings Result);
};
