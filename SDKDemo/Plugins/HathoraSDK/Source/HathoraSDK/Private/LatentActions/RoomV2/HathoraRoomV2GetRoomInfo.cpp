// Copyright 2023 Hathora, Inc.

#include "LatentActions/RoomV2/HathoraRoomV2GetRoomInfo.h"
#include "HathoraSDKModule.h"

UHathoraRoomV2GetRoomInfo *UHathoraRoomV2GetRoomInfo::GetRoomInfo(
	UHathoraSDKRoomV2 *HathoraSDKRoomV2,
	UObject *WorldContextObject,
	FString RoomId
) {
	UHathoraRoomV2GetRoomInfo *Action = NewObject<UHathoraRoomV2GetRoomInfo>();
	Action->HathoraSDKRoomV2 = HathoraSDKRoomV2;
	Action->RoomId = RoomId;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraRoomV2GetRoomInfo::Activate()
{
	if (!IsValid(this) || !IsValid(HathoraSDKRoomV2))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("GetRoomInfo failed because the underlying Hathora API is not valid."));

		if (IsValid(this))
		{
			SetReadyToDestroy();
		}

		return;
	}

	HathoraSDKRoomV2->GetRoomInfo(
		RoomId,
		UHathoraSDKRoomV2::FHathoraOnGetRoomInfo::CreateLambda(
			[this](const FHathoraGetRoomInfoResult& Result)
			{
				OnComplete.Broadcast(Result);
				SetReadyToDestroy();
			}
		)
	);
}
