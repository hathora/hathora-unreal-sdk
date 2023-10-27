// Copyright 2023 Hathora, Inc.
#pragma once

#include "CoreMinimal.h"
#include "HathoraTypes.h"
#include "HathoraSDKConfig.h"
#include "Delegates/Delegate.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HathoraSDK.generated.h"

class UHathoraSDKDiscoveryV1;

UCLASS()
class HATHORASDK_API UHathoraSDK : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Get ping times to all available Hathora Cloud regions
	// pings are returned in milliseconds
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK")
	static void GetRegionalPings(const FHathoraOnGetRegionalPings& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "HathoraSDK")
	static UHathoraSDK* CreateHathoraSDK(FString AppId, FHathoraSDKSecurity Security);

	UFUNCTION(BlueprintCallable, Category = "HathoraSDK")
	void SetCredentials(FString AppId, FHathoraSDKSecurity Security);

	UPROPERTY(BlueprintReadOnly)
	UHathoraSDKDiscoveryV1* DiscoveryV1;
};
