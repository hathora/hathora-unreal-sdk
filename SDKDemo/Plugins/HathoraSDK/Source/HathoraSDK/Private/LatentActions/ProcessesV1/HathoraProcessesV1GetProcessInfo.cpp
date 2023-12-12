// Copyright 2023 Hathora, Inc.

#include "LatentActions/ProcessesV1/HathoraProcessesV1GetProcessInfo.h"
#include "HathoraSDKModule.h"

UHathoraProcessesV1GetProcessInfo *UHathoraProcessesV1GetProcessInfo::GetProcessInfo(
	UHathoraSDKProcessesV1 *HathoraSDKProcessesV1,
	UObject *WorldContextObject,
	FString ProcessId
) {
	UHathoraProcessesV1GetProcessInfo *Action = NewObject<UHathoraProcessesV1GetProcessInfo>();
	Action->HathoraSDKProcessesV1 = HathoraSDKProcessesV1;
	Action->ProcessId = ProcessId;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraProcessesV1GetProcessInfo::Activate()
{
	if (!IsValid(this) || !IsValid(HathoraSDKProcessesV1))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("GetProcessInfo failed because the underlying Hathora API is not valid."));

		if (IsValid(this))
		{
			SetReadyToDestroy();
		}

		return;
	}

	HathoraSDKProcessesV1->GetProcessInfo(
		ProcessId,
		UHathoraSDKProcessesV1::FHathoraOnProcessInfo::CreateLambda(
			[this](const FHathoraProcessInfoResult& Result)
			{
				OnComplete.Broadcast(Result);
				SetReadyToDestroy();
			}
		)
	);
}
