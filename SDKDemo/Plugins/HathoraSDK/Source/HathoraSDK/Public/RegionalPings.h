// Copyright 2023 Hathora, Inc.
#pragma once

#include "RegionalPings.generated.h"

USTRUCT(BlueprintType)
struct HATHORASDK_API FRegionalPings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, int32> PingMap;
};
