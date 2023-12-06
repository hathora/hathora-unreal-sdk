// Copyright 2023 Hathora, Inc.

#include "LatentActions/ProcessesV1/HathoraProcessesV1GetAllRunningProcesses.h"
#include "HathoraSDKModule.h"

UHathoraProcessesV1GetAllRunningProcesses *UHathoraProcessesV1GetAllRunningProcesses::GetAllRunningProcesses(
	UHathoraSDKProcessesV1 *HathoraSDKProcessesV1,
	UObject *WorldContextObject
) {
	UHathoraProcessesV1GetAllRunningProcesses *Action = NewObject<UHathoraProcessesV1GetAllRunningProcesses>();
	Action->HathoraSDKProcessesV1 = HathoraSDKProcessesV1;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraProcessesV1GetAllRunningProcesses::Activate()
{
	if (!IsValid(this) || !IsValid(HathoraSDKProcessesV1))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("GetAllRunningProcesses failed because the underlying Hathora API is not valid."));

		if (IsValid(this))
		{
			SetReadyToDestroy();
		}

		return;
	}

	HathoraSDKProcessesV1->GetAllRunningProcesses(
		UHathoraSDKProcessesV1::FHathoraOnProcessInfos::CreateLambda(
			[this](const FHathoraProcessInfosResult& Result)
			{
				OnComplete.Broadcast(Result);
				SetReadyToDestroy();
			}
		)
	);
}
