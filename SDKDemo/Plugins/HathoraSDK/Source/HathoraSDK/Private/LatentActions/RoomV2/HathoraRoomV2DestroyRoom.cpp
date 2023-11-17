// Copyright 2023 Hathora, Inc.

#include "LatentActions/RoomV2/HathoraRoomV2DestroyRoom.h"

UHathoraRoomV2DestroyRoom *UHathoraRoomV2DestroyRoom::DestroyRoom(
	UHathoraSDKRoomV2 *HathoraSDKRoomV2,
	UObject *WorldContextObject,
	FString RoomId
) {
	UHathoraRoomV2DestroyRoom *Action = NewObject<UHathoraRoomV2DestroyRoom>();
	Action->HathoraSDKRoomV2 = HathoraSDKRoomV2;
	Action->RoomId = RoomId;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraRoomV2DestroyRoom::Activate()
{
	HathoraSDKRoomV2->DestroyRoom(
		RoomId,
		UHathoraSDKRoomV2::FHathoraOnDestroyRoom::CreateLambda(
			[this](const FHathoraDestroyRoomResult& Result)
			{
				OnComplete.Broadcast(Result);
				SetReadyToDestroy();
			}
		)
	);
}
