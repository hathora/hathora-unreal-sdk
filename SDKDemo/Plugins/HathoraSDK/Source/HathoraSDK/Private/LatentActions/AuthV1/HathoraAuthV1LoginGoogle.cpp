// Copyright 2023 Hathora, Inc.

#include "LatentActions/AuthV1/HathoraAuthV1LoginGoogle.h"
#include "HathoraSDKModule.h"

UHathoraAuthV1LoginGoogle *UHathoraAuthV1LoginGoogle::LoginGoogle(
	UHathoraSDKAuthV1 *HathoraSDKAuthV1,
	UObject *WorldContextObject,
	FString IdToken
) {
	UHathoraAuthV1LoginGoogle *Action = NewObject<UHathoraAuthV1LoginGoogle>();
	Action->HathoraSDKAuthV1 = HathoraSDKAuthV1;
	Action->IdToken = IdToken;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraAuthV1LoginGoogle::Activate()
{
	if (!IsValid(this) || !IsValid(HathoraSDKAuthV1))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("LoginGoogle failed because the underlying Hathora API is not valid."));

		if (IsValid(this))
		{
			SetReadyToDestroy();
		}

		return;
	}

	HathoraSDKAuthV1->LoginGoogle(
		IdToken,
		UHathoraSDKAuthV1::FHathoraOnLogin::CreateLambda(
			[this](const FHathoraLoginResult& Result)
			{
				OnComplete.Broadcast(Result);
				SetReadyToDestroy();
			}
		)
	);
}
