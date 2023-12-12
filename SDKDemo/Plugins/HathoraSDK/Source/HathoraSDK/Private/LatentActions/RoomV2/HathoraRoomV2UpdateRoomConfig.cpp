// Copyright 2023 Hathora, Inc.

#include "LatentActions/RoomV2/HathoraRoomV2UpdateRoomConfig.h"
#include "HathoraSDKModule.h"

UHathoraRoomV2UpdateRoomConfig *UHathoraRoomV2UpdateRoomConfig::UpdateRoomConfig(
	UHathoraSDKRoomV2 *HathoraSDKRoomV2,
	UObject *WorldContextObject,
	FString RoomId,
	FString RoomConfig
) {
	UHathoraRoomV2UpdateRoomConfig *Action = NewObject<UHathoraRoomV2UpdateRoomConfig>();
	Action->HathoraSDKRoomV2 = HathoraSDKRoomV2;
	Action->RoomId = RoomId;
	Action->RoomConfig = RoomConfig;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraRoomV2UpdateRoomConfig::Activate()
{
	if (!IsValid(this) || !IsValid(HathoraSDKRoomV2))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("UpdateRoomConfig failed because the underlying Hathora API is not valid."));

		if (IsValid(this))
		{
			SetReadyToDestroy();
		}

		return;
	}

	HathoraSDKRoomV2->UpdateRoomConfig(
		RoomId,
		RoomConfig,
		UHathoraSDKRoomV2::FHathoraOnUpdateRoomConfig::CreateLambda(
			[this](const FHathoraUpdateRoomConfigResult& Result)
			{
				OnComplete.Broadcast(Result);
				SetReadyToDestroy();
			}
		)
	);
}
