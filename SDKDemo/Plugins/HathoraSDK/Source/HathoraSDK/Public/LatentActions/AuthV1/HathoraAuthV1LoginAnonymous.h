// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HathoraTypes.h"
#include "HathoraAuthV1LatentCommon.h"
#include "HathoraAuthV1LoginAnonymous.generated.h"

UCLASS()
class HATHORASDK_API UHathoraAuthV1LoginAnonymous : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

	// Returns a unique player token for an anonymous user.
	UFUNCTION(
		BlueprintCallable,
		meta =
			(BlueprintInternalUseOnly = "true",
			 Category = "HathoraSDK | AuthV1",
			 WorldContext = "WorldContextObject")
	)
	static UHathoraAuthV1LoginAnonymous *LoginAnonymous(
		UHathoraSDKAuthV1 *HathoraSDKAuthV1,
		UObject *WorldContextObject
	);

	UPROPERTY()
	UHathoraSDKAuthV1 *HathoraSDKAuthV1;

	UPROPERTY(BlueprintAssignable)
	FHathoraAuthV1LoginComplete OnComplete;
};