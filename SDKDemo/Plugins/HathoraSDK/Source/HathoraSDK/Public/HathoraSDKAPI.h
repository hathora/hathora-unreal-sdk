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
	GENERATED_UCLASS_BODY()

public:
	void SetCredentials(FString AppId, FHathoraSDKSecurity Security);

	FString GetAuthToken();

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

	UFUNCTION()
	void OnWorldRemoved(ULevel* Level, UWorld* World);

	FString AppId;
	FHathoraSDKSecurity Security;

	bool bIsBoundToWorldRemoving = false;
	bool bWorldIsBeingDestroyed = false;
};
