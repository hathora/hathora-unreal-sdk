// Copyright 2023 Hathora, Inc.

#include "LatentActions/ProcessesV2/HathoraProcessesV2GetLatestProcesses.h"
#include "HathoraSDKModule.h"

UHathoraProcessesV2GetLatestProcesses *UHathoraProcessesV2GetLatestProcesses::GetLatestProcesses(
	UHathoraSDKProcessesV2 *HathoraSDKProcessesV2,
	UObject *WorldContextObject,
	TArray<EHathoraProcessStatus> StatusFilter,
	TArray<EHathoraCloudRegion> RegionFilter
) {
	UHathoraProcessesV2GetLatestProcesses *Action = NewObject<UHathoraProcessesV2GetLatestProcesses>();
	Action->HathoraSDKProcessesV2 = HathoraSDKProcessesV2;
	Action->StatusFilter = StatusFilter;
	Action->RegionFilter = RegionFilter;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraProcessesV2GetLatestProcesses::Activate()
{
	if (!IsValid(this) || !IsValid(HathoraSDKProcessesV2))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("GetLatestProcesses failed because the underlying Hathora API is not valid."));

		if (IsValid(this))
		{
			SetReadyToDestroy();
		}

		return;
	}

	HathoraSDKProcessesV2->GetLatestProcesses(
		StatusFilter,
		RegionFilter,
		UHathoraSDKProcessesV2::FHathoraOnProcessInfos::CreateLambda(
			[this](const FHathoraProcessInfosResult& Result)
			{
				OnComplete.Broadcast(Result);
				SetReadyToDestroy();
			}
		)
	);
}
