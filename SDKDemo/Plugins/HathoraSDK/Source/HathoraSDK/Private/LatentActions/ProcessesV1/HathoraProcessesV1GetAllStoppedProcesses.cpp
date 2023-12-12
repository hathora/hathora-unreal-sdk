// Copyright 2023 Hathora, Inc.

#include "LatentActions/ProcessesV1/HathoraProcessesV1GetAllStoppedProcesses.h"
#include "HathoraSDKModule.h"

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
	if (!IsValid(this) || !IsValid(HathoraSDKProcessesV1))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("GetAllStoppedProcesses failed because the underlying Hathora API is not valid."));

		if (IsValid(this))
		{
			SetReadyToDestroy();
		}

		return;
	}

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
