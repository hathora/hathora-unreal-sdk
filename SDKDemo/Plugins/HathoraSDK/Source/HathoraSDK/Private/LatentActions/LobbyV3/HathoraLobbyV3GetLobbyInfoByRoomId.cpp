// Copyright 2023 Hathora, Inc.

#include "LatentActions/LobbyV3/HathoraLobbyV3GetLobbyInfoByRoomId.h"
#include "HathoraSDKModule.h"

UHathoraLobbyV3GetLobbyInfoByRoomId *UHathoraLobbyV3GetLobbyInfoByRoomId::GetLobbyInfoByRoomId(
	UHathoraSDKLobbyV3 *HathoraSDKLobbyV3,
	UObject *WorldContextObject,
	FString RoomId
) {
	UHathoraLobbyV3GetLobbyInfoByRoomId *Action = NewObject<UHathoraLobbyV3GetLobbyInfoByRoomId>();
	Action->HathoraSDKLobbyV3 = HathoraSDKLobbyV3;
	Action->RoomId = RoomId;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraLobbyV3GetLobbyInfoByRoomId::Activate()
{
	if (!IsValid(this) || !IsValid(HathoraSDKLobbyV3))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("GetLobbyInfoByRoomId failed because the underlying Hathora API is not valid."));

		if (IsValid(this))
		{
			SetReadyToDestroy();
		}

		return;
	}

	HathoraSDKLobbyV3->GetLobbyInfoByRoomId(
		RoomId,
		UHathoraSDKLobbyV3::FHathoraOnLobbyInfo::CreateLambda(
			[this](const FHathoraLobbyInfoResult& Result)
			{
				OnComplete.Broadcast(Result);
				SetReadyToDestroy();
			}
		)
	);
}
