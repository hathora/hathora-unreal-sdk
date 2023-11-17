// Copyright 2023 Hathora, Inc.

#include "LatentActions/RoomV2/HathoraRoomV2UpdateRoomConfig.h"

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
