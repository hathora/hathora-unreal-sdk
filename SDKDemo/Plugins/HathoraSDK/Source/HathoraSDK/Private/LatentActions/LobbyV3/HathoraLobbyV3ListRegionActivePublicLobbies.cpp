// Copyright 2023 Hathora, Inc.

#include "LatentActions/LobbyV3/HathoraLobbyV3ListRegionActivePublicLobbies.h"
#include "HathoraSDKModule.h"

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
	if (!IsValid(this) || !IsValid(HathoraSDKLobbyV3))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("ListRegionActivePublicLobbies failed because the underlying Hathora API is not valid."));

		if (IsValid(this))
		{
			SetReadyToDestroy();
		}

		return;
	}

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
