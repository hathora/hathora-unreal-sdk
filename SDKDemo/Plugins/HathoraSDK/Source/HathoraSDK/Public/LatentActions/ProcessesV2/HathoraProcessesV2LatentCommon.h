// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "HathoraSDKProcessesV2.h"
#include "HathoraProcessesV2LatentCommon.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FHathoraProcessesV2OnProcessInfo, const FHathoraProcessInfoResult&, Result
);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FHathoraProcessesV2OnProcessInfos, const FHathoraProcessInfosResult&, Result
);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FHathoraProcessesV2OnStopProcess, const FHathoraStopProcessResult&, Result
);
