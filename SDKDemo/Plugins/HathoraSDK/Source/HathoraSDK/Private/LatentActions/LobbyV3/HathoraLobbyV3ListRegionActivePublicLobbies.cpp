// Copyright 2023 Hathora, Inc.

#include "LatentActions/LobbyV3/HathoraLobbyV3ListRegionActivePublicLobbies.h"

UHathoraLobbyV3ListRegionActivePublicLobbies *UHathoraLobbyV3ListRegionActivePublicLobbies::ListRegionActivePublicLobbies(
	UHathoraSDKLobbyV3 *HathoraSDKLobbyV3,
	UObject *WorldContextObject,
	EHathoraCloudRegion Region
) {
	UHathoraLobbyV3ListRegionActivePublicLobbies *Action = NewObject<UHathoraLobbyV3ListRegionActivePublicLobbies>();
	Action->HathoraSDKLobbyV3 = HathoraSDKLobbyV3;
	Action->Region = Region;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraLobbyV3ListRegionActivePublicLobbies::Activate()
{
	HathoraSDKLobbyV3->ListRegionActivePublicLobbies(
		Region,
		UHathoraSDKLobbyV3::FHathoraOnLobbyInfos::CreateLambda(
			[this](const FHathoraLobbyInfosResult& Result)
			{
				OnComplete.Broadcast(Result);
				SetReadyToDestroy();
			}
		)
	);
}
