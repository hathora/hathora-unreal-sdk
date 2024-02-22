// Copyright 2023 Hathora, Inc.

#include "LatentActions/ProcessesV2/HathoraProcessesV2GetProcessInfo.h"
#include "HathoraSDKModule.h"

UHathoraProcessesV2GetProcessInfo *UHathoraProcessesV2GetProcessInfo::GetProcessInfo(
	UHathoraSDKProcessesV2 *HathoraSDKProcessesV2,
	UObject *WorldContextObject,
	FString ProcessId
) {
	UHathoraProcessesV2GetProcessInfo *Action = NewObject<UHathoraProcessesV2GetProcessInfo>();
	Action->HathoraSDKProcessesV2 = HathoraSDKProcessesV2;
	Action->ProcessId = ProcessId;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UHathoraProcessesV2GetProcessInfo::Activate()
{
	if (!IsValid(this) || !IsValid(HathoraSDKProcessesV2))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("GetProcessInfo failed because the underlying Hathora API is not valid."));

		if (IsValid(this))
		{
			SetReadyToDestroy();
		}

		return;
	}

	HathoraSDKProcessesV2->GetProcessInfo(
		ProcessId,
		UHathoraSDKProcessesV2::FHathoraOnProcessInfo::CreateLambda(
			[this](const FHathoraProcessInfoResult& Result)
			{
				OnComplete.Broadcast(Result);
				SetReadyToDestroy();
			}
		)
	);
}
