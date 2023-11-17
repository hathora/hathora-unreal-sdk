// Copyright 2023 Hathora, Inc.

#include "LatentActions/RoomV2/HathoraRoomV2CreateRoom.h"

UHathoraRoomV2CreateRoom *UHathoraRoomV2CreateRoom::CreateRoom(
	UHathoraSDKRoomV2 *HathoraSDKRoomV2,
	UObject *WorldContextObject,
	EHathoraCloudRegion Region,
	FString RoomConfig,
	FString RoomId
) {
	UHathoraRoomV2CreateRoom *Action = NewObject<UHathoraRoomV2CreateRoom>();
	Action->HathoraSDKRoomV2 = HathoraSDKRoomV2;
	Action->Region = Region;
	Action->RoomConfig = RoomConfig;
	Action->RoomId = RoomId;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraRoomV2CreateRoom::Activate()
{
	HathoraSDKRoomV2->CreateRoom(
		Region,
		RoomConfig,
		RoomId,
		UHathoraSDKRoomV2::FHathoraOnRoomConnectionInfo::CreateLambda(
			[this](const FHathoraRoomConnectionInfoResult& Result)
			{
				OnComplete.Broadcast(Result);
				SetReadyToDestroy();
			}
		)
	);
}
