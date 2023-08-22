// Copyright 2023 Hathora, Inc.
#pragma once

#include "HathoraSDKTypes.generated.h"

USTRUCT(BlueprintType)
struct HATHORASDK_API FDiscoveredPingEndpoint
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString Region;

	UPROPERTY(BlueprintReadOnly)
	FString Host;

	UPROPERTY(BlueprintReadOnly)
	int32 Port = 0;
};

USTRUCT(BlueprintType)
struct HATHORASDK_API FRegionalPings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, int32> PingMap;
};
