// Copyright 2023 Hathora, Inc.

#include "LatentActions/AuthV1/HathoraAuthV1LoginAnonymous.h"
#include "HathoraSDKModule.h"

UHathoraAuthV1LoginAnonymous *UHathoraAuthV1LoginAnonymous::LoginAnonymous(
	UHathoraSDKAuthV1 *HathoraSDKAuthV1,
	UObject *WorldContextObject
) {
	UHathoraAuthV1LoginAnonymous *Action = NewObject<UHathoraAuthV1LoginAnonymous>();
	Action->HathoraSDKAuthV1 = HathoraSDKAuthV1;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraAuthV1LoginAnonymous::Activate()
{
	if (!IsValid(this) || !IsValid(HathoraSDKAuthV1))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("LoginAnonymous failed because the underlying Hathora API is not valid."));

		if (IsValid(this))
		{
			SetReadyToDestroy();
		}

		return;
	}

	HathoraSDKAuthV1->LoginAnonymous(
		UHathoraSDKAuthV1::FHathoraOnLogin::CreateLambda(
			[this](const FHathoraLoginResult& Result)
			{
				OnComplete.Broadcast(Result);
				SetReadyToDestroy();
			}
		)
	);
}
