// Copyright 2023 Hathora, Inc.

#include "LatentActions/LobbyV3/HathoraLobbyV3ListAllActivePublicLobbies.h"

UHathoraLobbyV3ListAllActivePublicLobbies *UHathoraLobbyV3ListAllActivePublicLobbies::ListAllActivePublicLobbies(
	UHathoraSDKLobbyV3 *HathoraSDKLobbyV3,
	UObject *WorldContextObject
) {
	UHathoraLobbyV3ListAllActivePublicLobbies *Action = NewObject<UHathoraLobbyV3ListAllActivePublicLobbies>();
	Action->HathoraSDKLobbyV3 = HathoraSDKLobbyV3;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraLobbyV3ListAllActivePublicLobbies::Activate()
{
	HathoraSDKLobbyV3->ListAllActivePublicLobbies(
		UHathoraSDKLobbyV3::FHathoraOnLobbyInfos::CreateLambda(
			[this](const FHathoraLobbyInfosResult& Result)
			{
				OnComplete.Broadcast(Result);
				SetReadyToDestroy();
			}
		)
	);
}
