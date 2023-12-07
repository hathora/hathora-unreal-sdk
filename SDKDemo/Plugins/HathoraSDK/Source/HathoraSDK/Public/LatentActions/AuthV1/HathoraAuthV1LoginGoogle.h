// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HathoraTypes.h"
#include "HathoraAuthV1LatentCommon.h"
#include "HathoraAuthV1LoginGoogle.generated.h"

UCLASS()
class HATHORASDK_API UHathoraAuthV1LoginGoogle : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

	// Returns a unique player token using a Google-signed OIDC idToken.
	// @param IdToken A Google-signed OIDC ID token representing a player's
	//                authenticated identity. Learn how to get an idToken
	//                [here](https://cloud.google.com/docs/authentication/get-id-token).
	UFUNCTION(
		BlueprintCallable,
		meta =
			(BlueprintInternalUseOnly = "true",
			 Category = "HathoraSDK | AuthV1",
			 WorldContext = "WorldContextObject")
	)
	static UHathoraAuthV1LoginGoogle *LoginGoogle(
		UHathoraSDKAuthV1 *HathoraSDKAuthV1,
		UObject *WorldContextObject,
		FString IdToken
	);

	UPROPERTY()
	UHathoraSDKAuthV1 *HathoraSDKAuthV1;

	FString IdToken;

	UPROPERTY(BlueprintAssignable)
	FHathoraAuthV1LoginComplete OnComplete;
};