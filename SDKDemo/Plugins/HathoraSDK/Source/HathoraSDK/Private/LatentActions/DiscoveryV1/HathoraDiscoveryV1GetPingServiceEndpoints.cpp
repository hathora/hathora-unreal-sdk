// Copyright 2023 Hathora, Inc.

#include "LatentActions/DiscoveryV1/HathoraDiscoveryV1GetPingServiceEndpoints.h"
#include "HathoraSDKModule.h"

UHathoraDiscoveryV1GetPingServiceEndpoints *UHathoraDiscoveryV1GetPingServiceEndpoints::GetPingServiceEndpoints(
	UHathoraSDKDiscoveryV1 *HathoraSDKDiscoveryV1,
	UObject *WorldContextObject
) {
	UHathoraDiscoveryV1GetPingServiceEndpoints *Action = NewObject<UHathoraDiscoveryV1GetPingServiceEndpoints>();
	Action->HathoraSDKDiscoveryV1 = HathoraSDKDiscoveryV1;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraDiscoveryV1GetPingServiceEndpoints::Activate()
{
	if (!IsValid(this) || !IsValid(HathoraSDKDiscoveryV1))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("GetPingServiceEndpoints failed because the underlying Hathora API is not valid."));

		if (IsValid(this))
		{
			SetReadyToDestroy();
		}

		return;
	}

	HathoraSDKDiscoveryV1->GetPingServiceEndpoints(
		UHathoraSDKDiscoveryV1::FHathoraOnGetPingServiceEndpoints::CreateLambda(
			[this](const TArray<FHathoraDiscoveredPingEndpoint>& Endpoints)
			{
				OnComplete.Broadcast(Endpoints);
				SetReadyToDestroy();
			}
		)
	);
}
