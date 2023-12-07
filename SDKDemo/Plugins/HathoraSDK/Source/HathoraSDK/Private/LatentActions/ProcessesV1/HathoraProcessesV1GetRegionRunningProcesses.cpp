// Copyright 2023 Hathora, Inc.

#include "LatentActions/ProcessesV1/HathoraProcessesV1GetRegionRunningProcesses.h"

UHathoraProcessesV1GetRegionRunningProcesses *UHathoraProcessesV1GetRegionRunningProcesses::GetRegionRunningProcesses(
	UHathoraSDKProcessesV1 *HathoraSDKProcessesV1,
	UObject *WorldContextObject,
	EHathoraCloudRegion Region
) {
	UHathoraProcessesV1GetRegionRunningProcesses *Action = NewObject<UHathoraProcessesV1GetRegionRunningProcesses>();
	Action->HathoraSDKProcessesV1 = HathoraSDKProcessesV1;
	Action->Region = Region;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraProcessesV1GetRegionRunningProcesses::Activate()
{
	HathoraSDKProcessesV1->GetRegionRunningProcesses(
		Region,
		UHathoraSDKProcessesV1::FHathoraOnProcessInfos::CreateLambda(
			[this](const FHathoraProcessInfosResult& Result)
			{
				OnComplete.Broadcast(Result);
				SetReadyToDestroy();
			}
		)
	);
}
