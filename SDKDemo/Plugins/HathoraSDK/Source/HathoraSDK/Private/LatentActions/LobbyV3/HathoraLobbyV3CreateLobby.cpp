// Copyright 2023 Hathora, Inc.

#include "LatentActions/LobbyV3/HathoraLobbyV3CreateLobby.h"
#include "HathoraSDKModule.h"

UHathoraLobbyV3CreateLobby *UHathoraLobbyV3CreateLobby::CreateLobby(
	UHathoraSDKLobbyV3 *HathoraSDKLobbyV3,
	UObject *WorldContextObject,
	EHathoraLobbyVisibility Visibility,
	FString RoomConfig,
	EHathoraCloudRegion Region,
	FString ShortCode,
	FString RoomId
) {
	UHathoraLobbyV3CreateLobby *Action = NewObject<UHathoraLobbyV3CreateLobby>();
	Action->HathoraSDKLobbyV3 = HathoraSDKLobbyV3;
	Action->Visibility = Visibility;
	Action->RoomConfig = RoomConfig;
	Action->Region = Region;
	Action->ShortCode = ShortCode;
	Action->RoomId = RoomId;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraLobbyV3CreateLobby::Activate()
{
	if (!IsValid(this) || !IsValid(HathoraSDKLobbyV3))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("CreateLobby failed because the underlying Hathora API is not valid."));

		if (IsValid(this))
		{
			SetReadyToDestroy();
		}

		return;
	}

	HathoraSDKLobbyV3->CreateLobby(
		Visibility,
		RoomConfig,
		Region,
		ShortCode,
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
