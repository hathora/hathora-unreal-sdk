// Copyright 2023 Hathora, Inc.

#include "LatentActions/ProcessesV1/HathoraProcessesV1GetAllStoppedProcesses.h"

UHathoraProcessesV1GetAllStoppedProcesses *UHathoraProcessesV1GetAllStoppedProcesses::GetAllStoppedProcesses(
	UHathoraSDKProcessesV1 *HathoraSDKProcessesV1,
	UObject *WorldContextObject
) {
	UHathoraProcessesV1GetAllStoppedProcesses *Action = NewObject<UHathoraProcessesV1GetAllStoppedProcesses>();
	Action->HathoraSDKProcessesV1 = HathoraSDKProcessesV1;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraProcessesV1GetAllStoppedProcesses::Activate()
{
	HathoraSDKProcessesV1->GetAllStoppedProcesses(
		UHathoraSDKProcessesV1::FHathoraOnProcessInfos::CreateLambda(
			[this](const FHathoraProcessInfosResult& Result)
			{
				OnComplete.Broadcast(Result);
				SetReadyToDestroy();
			}
		)
	);
}
