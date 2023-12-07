// Copyright 2023 Hathora, Inc.

#include "LatentActions/RoomV2/HathoraRoomV2GetActiveRoomsForProcess.h"

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
