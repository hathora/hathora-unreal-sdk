// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "HathoraSDKProcessesV1.h"
#include "HathoraProcessesV1LatentCommon.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FHathoraProcessesV1OnProcessInfo, const FHathoraProcessInfoResult&, Result
);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FHathoraProcessesV1OnProcessInfos, const FHathoraProcessInfosResult&, Result
);
