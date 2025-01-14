// Copyright 2023 Hathora, Inc.

#include "LatentActions/HathoraGetPingsForRegions.h"
#include "HathoraSDK.h"
#include "HathoraSDKModule.h"

UHathoraGetPingsForRegions *UHathoraGetPingsForRegions::GetPingsForRegions(
	UObject *WorldContextObject,
	TMap<FString, FString> RegionUrls,
	EHathoraPingType PingType,
	int32 NumPingsPerRegion
) {
	UHathoraGetPingsForRegions *Action = NewObject<UHathoraGetPingsForRegions>();
	Action->RegionUrls = RegionUrls;
	Action->PingType = PingType;
	Action->NumPingsPerRegion = NumPingsPerRegion;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraGetPingsForRegions::Activate()
{
	if (!IsValid(this))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("GetPingsForRegions failed because the underlying Hathora API is not valid."));
		return;
	}

	UHathoraSDK::GetPingsForRegions(
		RegionUrls,
		PingType,
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
