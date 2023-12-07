// Copyright 2023 Hathora, Inc.

#include "LatentActions/ProcessesV1/HathoraProcessesV1GetRegionStoppedProcesses.h"

UHathoraProcessesV1GetRegionStoppedProcesses *UHathoraProcessesV1GetRegionStoppedProcesses::GetRegionStoppedProcesses(
	UHathoraSDKProcessesV1 *HathoraSDKProcessesV1,
	UObject *WorldContextObject,
	EHathoraCloudRegion Region
) {
	UHathoraProcessesV1GetRegionStoppedProcesses *Action = NewObject<UHathoraProcessesV1GetRegionStoppedProcesses>();
	Action->HathoraSDKProcessesV1 = HathoraSDKProcessesV1;
	Action->Region = Region;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraProcessesV1GetRegionStoppedProcesses::Activate()
{
	HathoraSDKProcessesV1->GetRegionStoppedProcesses(
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
