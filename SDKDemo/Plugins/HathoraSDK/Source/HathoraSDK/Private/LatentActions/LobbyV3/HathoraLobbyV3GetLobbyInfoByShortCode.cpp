// Copyright 2023 Hathora, Inc.

#include "LatentActions/LobbyV3/HathoraLobbyV3GetLobbyInfoByShortCode.h"

UHathoraLobbyV3GetLobbyInfoByShortCode *UHathoraLobbyV3GetLobbyInfoByShortCode::GetLobbyInfoByShortCode(
	UHathoraSDKLobbyV3 *HathoraSDKLobbyV3,
	UObject *WorldContextObject,
	FString ShortCode
) {
	UHathoraLobbyV3GetLobbyInfoByShortCode *Action = NewObject<UHathoraLobbyV3GetLobbyInfoByShortCode>();
	Action->HathoraSDKLobbyV3 = HathoraSDKLobbyV3;
	Action->ShortCode = ShortCode;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraLobbyV3GetLobbyInfoByShortCode::Activate()
{
	HathoraSDKLobbyV3->GetLobbyInfoByShortCode(
		ShortCode,
		UHathoraSDKLobbyV3::FHathoraOnLobbyInfo::CreateLambda(
			[this](const FHathoraLobbyInfoResult& Result)
			{
				OnComplete.Broadcast(Result);
				SetReadyToDestroy();
			}
		)
	);
}
