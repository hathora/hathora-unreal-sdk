// Copyright 2023 Hathora, Inc.

#include "LatentActions/ProcessesV1/HathoraProcessesV1GetRegionStoppedProcesses.h"
#include "HathoraSDKModule.h"

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
	if (!IsValid(this) || !IsValid(HathoraSDKProcessesV1))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("GetRegionStoppedProcesses failed because the underlying Hathora API is not valid."));

		if (IsValid(this))
		{
			SetReadyToDestroy();
		}

		return;
	}

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
