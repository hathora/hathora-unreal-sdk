// Copyright 2023 Hathora, Inc.

#include "LatentActions/LobbyV3/HathoraLobbyV3GetLobbyInfoByShortCode.h"
#include "HathoraSDKModule.h"

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
	if (!IsValid(this) || !IsValid(HathoraSDKLobbyV3))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("GetLobbyInfoByShortCode failed because the underlying Hathora API is not valid."));

		if (IsValid(this))
		{
			SetReadyToDestroy();
		}

		return;
	}

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
