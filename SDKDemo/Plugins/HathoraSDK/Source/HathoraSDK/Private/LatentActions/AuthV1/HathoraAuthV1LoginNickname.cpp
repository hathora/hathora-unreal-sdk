// Copyright 2023 Hathora, Inc.

#include "LatentActions/AuthV1/HathoraAuthV1LoginNickname.h"
#include "HathoraSDKModule.h"

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
	if (!IsValid(this) || !IsValid(HathoraSDKAuthV1))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("LoginNickname failed because the underlying Hathora API is not valid."));

		if (IsValid(this))
		{
			SetReadyToDestroy();
		}

		return;
	}

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
