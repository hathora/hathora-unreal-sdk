// Copyright 2023 Hathora, Inc.

#include "LatentActions/LobbyV3/HathoraLobbyV3ListAllActivePublicLobbies.h"
#include "HathoraSDKModule.h"

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
	if (!IsValid(this) || !IsValid(HathoraSDKLobbyV3))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("ListAllActivePublicLobbies failed because the underlying Hathora API is not valid."));

		if (IsValid(this))
		{
			SetReadyToDestroy();
		}

		return;
	}

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
