// Copyright 2023 Hathora, Inc.

#include "LatentActions/HathoraGetRegionalPings.h"
#include "HathoraSDK.h"

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
	UHathoraSDK::GetRegionalPings(
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
