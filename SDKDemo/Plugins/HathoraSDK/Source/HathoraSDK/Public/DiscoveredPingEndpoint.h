// Copyright 2023 Hathora, Inc.
#pragma once

#include "DiscoveredPingEndpoint.generated.h"

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
