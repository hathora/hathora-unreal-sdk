// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HathoraTypes.h"
#include "HathoraAuthV1LatentCommon.h"
#include "HathoraAuthV1LoginNickname.generated.h"

UCLASS()
class HATHORASDK_API UHathoraAuthV1LoginNickname : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

	// Returns a unique player token with a specified nickname for a user.
	// @param Nickname An alias to represent a player.
	UFUNCTION(
		BlueprintCallable,
		meta =
			(BlueprintInternalUseOnly = "true",
			 Category = "HathoraSDK | AuthV1",
			 WorldContext = "WorldContextObject")
	)
	static UHathoraAuthV1LoginNickname *LoginNickname(
		UHathoraSDKAuthV1 *HathoraSDKAuthV1,
		UObject *WorldContextObject,
		FString Nickname
	);

	UPROPERTY()
	UHathoraSDKAuthV1 *HathoraSDKAuthV1;

	FString Nickname;

	UPROPERTY(BlueprintAssignable)
	FHathoraAuthV1LoginComplete OnComplete;
};