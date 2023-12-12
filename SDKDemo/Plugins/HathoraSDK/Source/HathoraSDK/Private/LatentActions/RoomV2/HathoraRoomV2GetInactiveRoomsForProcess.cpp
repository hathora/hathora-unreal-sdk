// Copyright 2023 Hathora, Inc.

#include "LatentActions/RoomV2/HathoraRoomV2GetInactiveRoomsForProcess.h"
#include "HathoraSDKModule.h"

UHathoraRoomV2GetInactiveRoomsForProcess *UHathoraRoomV2GetInactiveRoomsForProcess::GetInactiveRoomsForProcess(
	UHathoraSDKRoomV2 *HathoraSDKRoomV2,
	UObject *WorldContextObject,
	FString ProcessId
) {
	UHathoraRoomV2GetInactiveRoomsForProcess *Action = NewObject<UHathoraRoomV2GetInactiveRoomsForProcess>();
	Action->HathoraSDKRoomV2 = HathoraSDKRoomV2;
	Action->ProcessId = ProcessId;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraRoomV2GetInactiveRoomsForProcess::Activate()
{
	if (!IsValid(this) || !IsValid(HathoraSDKRoomV2))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("GetInactiveRoomsForProcess failed because the underlying Hathora API is not valid."));

		if (IsValid(this))
		{
			SetReadyToDestroy();
		}

		return;
	}

	HathoraSDKRoomV2->GetInactiveRoomsForProcess(
		ProcessId,
		UHathoraSDKRoomV2::FHathoraOnGetRoomsForProcess::CreateLambda(
			[this](const FHathoraGetRoomsForProcessResult& Result)
			{
				OnComplete.Broadcast(Result);
				SetReadyToDestroy();
			}
		)
	);
}
