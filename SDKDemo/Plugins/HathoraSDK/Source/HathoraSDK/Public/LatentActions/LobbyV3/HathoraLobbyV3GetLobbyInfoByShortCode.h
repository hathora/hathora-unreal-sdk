// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HathoraTypes.h"
#include "HathoraLobbyV3LatentCommon.h"
#include "HathoraLobbyV3GetLobbyInfoByShortCode.generated.h"

UCLASS()
class HATHORASDK_API UHathoraLobbyV3GetLobbyInfoByShortCode : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

	// Get details for a lobby. If 2 or more lobbies have the same shortCode, then the most
	// recently created lobby will be returned.
	// @param ShortCode User-defined identifier for a lobby.
	UFUNCTION(
		BlueprintCallable,
		meta =
			(BlueprintInternalUseOnly = "true",
			 Category = "HathoraSDK | LobbyV3",
			 WorldContext = "WorldContextObject")
	)
	static UHathoraLobbyV3GetLobbyInfoByShortCode *GetLobbyInfoByShortCode(
		UHathoraSDKLobbyV3 *HathoraSDKLobbyV3,
		UObject *WorldContextObject,
		FString ShortCode
	);

	UPROPERTY(BlueprintAssignable)
	FHathoraLobbyV3OnLobbyInfo OnComplete;

	UPROPERTY()
	UHathoraSDKLobbyV3 *HathoraSDKLobbyV3;

	FString ShortCode;
};