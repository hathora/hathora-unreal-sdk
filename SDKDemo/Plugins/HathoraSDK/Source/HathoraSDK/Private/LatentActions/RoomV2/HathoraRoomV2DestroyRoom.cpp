// Copyright 2023 Hathora, Inc.

#include "LatentActions/RoomV2/HathoraRoomV2DestroyRoom.h"
#include "HathoraSDKModule.h"

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
	if (!IsValid(this) || !IsValid(HathoraSDKRoomV2))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("DestroyRoom failed because the underlying Hathora API is not valid."));

		if (IsValid(this))
		{
			SetReadyToDestroy();
		}

		return;
	}

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
