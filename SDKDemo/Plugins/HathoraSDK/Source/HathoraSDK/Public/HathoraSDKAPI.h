// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HathoraTypes.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "HathoraSDKAPI.generated.h"

UCLASS(BlueprintType)
class HATHORASDK_API UHathoraSDKAPI : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK")
	void SetCredentials(FString AppId, FHathoraSDKSecurity Security);

protected:
	void SendRequest(
		FString Method,
		FString Endpoint,
		TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> OnProcessRequestComplete
	);

	void SendRequest(
		FString Method,
		FString Endpoint,
		TArray<TPair<FString, FString>> QueryOptions,
		TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> OnProcessRequestComplete
	);

	void SendRequest(
		FString Method,
		FString Endpoint,
		FJsonObject Body,
		TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> OnProcessRequestComplete
	);

	void SendRequest(
		FString Method,
		FString Endpoint,
		TArray<TPair<FString, FString>> QueryOptions,
		FJsonObject Body,
		TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> OnProcessRequestComplete
	);

	static FString GetRegionString(EHathoraCloudRegion Region);
	static EHathoraCloudRegion ParseRegion(FString RegionString);

	FString AppId;
	FHathoraSDKSecurity Security;
};
