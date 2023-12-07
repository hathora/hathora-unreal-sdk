// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "HathoraSDKLobbyV3.h"
#include "HathoraLobbyV3LatentCommon.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FHathoraLobbyV3OnLobbyInfo, const FHathoraLobbyInfoResult&, Result
);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FHathoraLobbyV3OnLobbyInfos, const FHathoraLobbyInfosResult&, Result
);
