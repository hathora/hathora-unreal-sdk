// Copyright 2023 Hathora, Inc.

#include "LatentActions/HathoraGetRegionalPings.h"
#include "HathoraSDK.h"
#include "HathoraSDKModule.h"

UHathoraGetRegionalPings *UHathoraGetRegionalPings::GetRegionalPings(
	UObject *WorldContextObject,
	int32 NumPingsPerRegion
) {
	UHathoraGetRegionalPings *Action = NewObject<UHathoraGetRegionalPings>();
	Action->NumPingsPerRegion = NumPingsPerRegion;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraGetRegionalPings::Activate()
{
	if (!IsValid(this))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("GetRegionalPings failed because the underlying Hathora API is not valid."));
		return;
	}

	UHathoraSDK::Internal_GetRegionalPings(
		FHathoraOnGetRegionalPings::CreateLambda(
			[this](const FHathoraRegionPings& Result)
			{
				OnComplete.Broadcast(Result);
				SetReadyToDestroy();
			}
		),
		NumPingsPerRegion
	);
}
