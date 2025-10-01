// Copyright 2023 Hathora, Inc.

#include "LatentActions/HathoraGetServerRoomId.h"
#include "HathoraSDK.h"
#include "HathoraSDKModule.h"

UHathoraGetServerRoomId *UHathoraGetServerRoomId::GetServerRoomId(
	UObject *WorldContextObject,
	float PollingInterval
) {
	UHathoraGetServerRoomId *Action = NewObject<UHathoraGetServerRoomId>();
	Action->PollingInterval = PollingInterval;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraGetServerRoomId::Activate()
{
	if (!IsValid(this))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("GetServerRoomId failed because the underlying Hathora API is not valid."));
		return;
	}

	UHathoraSDK::GetServerRoomId(
		PollingInterval,
		UHathoraSDK::FOnGetRoomId::CreateLambda(
			[this](const FString& RoomId)
			{
				OnComplete.Broadcast(RoomId);
				SetReadyToDestroy();
			}
		)
	);
}
