// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "HathoraSDKAuthV1.h"
#include "HathoraAuthV1LatentCommon.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FHathoraAuthV1LoginComplete, FHathoraLoginResult, Result
);
