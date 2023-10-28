// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "HathoraTypes.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "HathoraSDKAPI.generated.h"

UCLASS()
class HATHORASDK_API UHathoraSDKAPI : public UObject
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

	FString AppId;
	FHathoraSDKSecurity Security;
};
