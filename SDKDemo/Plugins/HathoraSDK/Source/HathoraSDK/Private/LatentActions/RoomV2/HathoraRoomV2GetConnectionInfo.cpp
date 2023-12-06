// Copyright 2023 Hathora, Inc.

#include "LatentActions/RoomV2/HathoraRoomV2GetConnectionInfo.h"
#include "HathoraSDKModule.h"

UHathoraRoomV2GetConnectionInfo *UHathoraRoomV2GetConnectionInfo::GetConnectionInfo(
	UHathoraSDKRoomV2 *HathoraSDKRoomV2,
	UObject *WorldContextObject,
	FString RoomId
) {
	UHathoraRoomV2GetConnectionInfo *Action = NewObject<UHathoraRoomV2GetConnectionInfo>();
	Action->HathoraSDKRoomV2 = HathoraSDKRoomV2;
	Action->RoomId = RoomId;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraRoomV2GetConnectionInfo::Activate()
{
	if (!IsValid(this) || !IsValid(HathoraSDKRoomV2))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("GetConnectionInfo failed because the underlying Hathora API is not valid."));

		if (IsValid(this))
		{
			SetReadyToDestroy();
		}

		return;
	}

	HathoraSDKRoomV2->GetConnectionInfo(
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
