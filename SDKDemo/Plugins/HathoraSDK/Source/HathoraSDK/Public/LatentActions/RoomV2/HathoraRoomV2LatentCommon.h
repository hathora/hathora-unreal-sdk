// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "HathoraSDKRoomV2.h"
#include "HathoraRoomV2LatentCommon.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FHathoraRoomV2OnRoomConnectionInfo, const FHathoraRoomConnectionInfoResult&, Result
);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FHathoraRoomV2OnGetRoomInfo, const FHathoraGetRoomInfoResult&, Result
);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FHathoraRoomV2OnGetRoomsForProcess, const FHathoraGetRoomsForProcessResult&, Result
);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FHathoraRoomV2OnDestroyRoom, const FHathoraDestroyRoomResult&, Result
);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FHathoraRoomV2OnSuspendRoom, const FHathoraSuspendRoomResult&, Result
);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FHathoraRoomV2OnUpdateRoomConfig, const FHathoraUpdateRoomConfigResult&, Result
);
