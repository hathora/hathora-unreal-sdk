// Copyright 2023 Hathora, Inc.

#include "LatentActions/RoomV2/HathoraRoomV2SuspendRoom.h"

UHathoraRoomV2SuspendRoom *UHathoraRoomV2SuspendRoom::SuspendRoom(
	UHathoraSDKRoomV2 *HathoraSDKRoomV2,
	UObject *WorldContextObject,
	FString RoomId
) {
	UHathoraRoomV2SuspendRoom *Action = NewObject<UHathoraRoomV2SuspendRoom>();
	Action->HathoraSDKRoomV2 = HathoraSDKRoomV2;
	Action->RoomId = RoomId;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraRoomV2SuspendRoom::Activate()
{
	HathoraSDKRoomV2->SuspendRoom(
		RoomId,
		UHathoraSDKRoomV2::FHathoraOnSuspendRoom::CreateLambda(
			[this](const FHathoraSuspendRoomResult& Result)
			{
				OnComplete.Broadcast(Result);
				SetReadyToDestroy();
			}
		)
	);
}
