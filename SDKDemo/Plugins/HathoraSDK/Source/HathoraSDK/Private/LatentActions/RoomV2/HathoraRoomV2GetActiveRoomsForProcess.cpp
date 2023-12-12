// Copyright 2023 Hathora, Inc.

#include "LatentActions/RoomV2/HathoraRoomV2GetActiveRoomsForProcess.h"
#include "HathoraSDKModule.h"

UHathoraRoomV2GetActiveRoomsForProcess *UHathoraRoomV2GetActiveRoomsForProcess::GetActiveRoomsForProcess(
	UHathoraSDKRoomV2 *HathoraSDKRoomV2,
	UObject *WorldContextObject,
	FString ProcessId
) {
	UHathoraRoomV2GetActiveRoomsForProcess *Action = NewObject<UHathoraRoomV2GetActiveRoomsForProcess>();
	Action->HathoraSDKRoomV2 = HathoraSDKRoomV2;
	Action->ProcessId = ProcessId;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraRoomV2GetActiveRoomsForProcess::Activate()
{
	if (!IsValid(this) || !IsValid(HathoraSDKRoomV2))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("GetActiveRoomsForProcess failed because the underlying Hathora API is not valid."));

		if (IsValid(this))
		{
			SetReadyToDestroy();
		}

		return;
	}

	HathoraSDKRoomV2->GetActiveRoomsForProcess(
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
