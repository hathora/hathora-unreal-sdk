// Copyright 2023 Hathora, Inc.

#include "LatentActions/DiscoveryV2/HathoraDiscoveryV2GetPingServiceEndpoints.h"
#include "HathoraSDKModule.h"

UHathoraDiscoveryV2GetPingServiceEndpoints *UHathoraDiscoveryV2GetPingServiceEndpoints::GetPingServiceEndpoints(
	UHathoraSDKDiscoveryV2 *HathoraSDKDiscoveryV2,
	UObject *WorldContextObject
) {
	UHathoraDiscoveryV2GetPingServiceEndpoints *Action = NewObject<UHathoraDiscoveryV2GetPingServiceEndpoints>();
	Action->HathoraSDKDiscoveryV2 = HathoraSDKDiscoveryV2;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraDiscoveryV2GetPingServiceEndpoints::Activate()
{
	if (!IsValid(this) || !IsValid(HathoraSDKDiscoveryV2))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("GetPingServiceEndpoints failed because the underlying Hathora API is not valid."));

		if (IsValid(this))
		{
			SetReadyToDestroy();
		}

		return;
	}

	HathoraSDKDiscoveryV2->GetPingServiceEndpoints(
		UHathoraSDKDiscoveryV2::FHathoraOnGetPingServiceEndpoints::CreateLambda(
			[this](const TArray<FHathoraDiscoveredPingEndpoint>& Endpoints)
			{
				OnComplete.Broadcast(Endpoints);
				SetReadyToDestroy();
			}
		)
	);
}
