// Copyright 2023 Hathora, Inc.

#include "LatentActions/ProcessesV2/HathoraProcessesV2StopProcess.h"
#include "HathoraSDKModule.h"

UHathoraProcessesV2StopProcess *UHathoraProcessesV2StopProcess::StopProcess(
	UHathoraSDKProcessesV2 *HathoraSDKProcessesV2,
	UObject *WorldContextObject,
	FString ProcessId
) {
	UHathoraProcessesV2StopProcess *Action = NewObject<UHathoraProcessesV2StopProcess>();
	Action->HathoraSDKProcessesV2 = HathoraSDKProcessesV2;
	Action->ProcessId = ProcessId;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraProcessesV2StopProcess::Activate()
{
	if (!IsValid(this) || !IsValid(HathoraSDKProcessesV2))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("StopProcess failed because the underlying Hathora API is not valid."));

		if (IsValid(this))
		{
			SetReadyToDestroy();
		}

		return;
	}

	HathoraSDKProcessesV2->StopProcess(
		ProcessId,
		UHathoraSDKProcessesV2::FHathoraOnStopProcess::CreateLambda(
			[this](const FHathoraStopProcessResult& Result)
			{
				OnComplete.Broadcast(Result);
				SetReadyToDestroy();
			}
		)
	);
}
