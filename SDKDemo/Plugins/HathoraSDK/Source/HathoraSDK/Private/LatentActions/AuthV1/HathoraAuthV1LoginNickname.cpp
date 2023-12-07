// Copyright 2023 Hathora, Inc.

#include "LatentActions/AuthV1/HathoraAuthV1LoginNickname.h"

UHathoraAuthV1LoginNickname *UHathoraAuthV1LoginNickname::LoginNickname(
	UHathoraSDKAuthV1 *HathoraSDKAuthV1,
	UObject *WorldContextObject,
	FString Nickname
) {
	UHathoraAuthV1LoginNickname *Action = NewObject<UHathoraAuthV1LoginNickname>();
	Action->HathoraSDKAuthV1 = HathoraSDKAuthV1;
	Action->Nickname = Nickname;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraAuthV1LoginNickname::Activate()
{
	HathoraSDKAuthV1->LoginNickname(
		Nickname,
		UHathoraSDKAuthV1::FHathoraOnLogin::CreateLambda(
			[this](const FHathoraLoginResult& Result)
			{
				OnComplete.Broadcast(Result);
				SetReadyToDestroy();
			}
		)
	);
}
