// Copyright 2023 Hathora, Inc.
#pragma once
#include "CoreMinimal.h"
#include "DiscoveredPingEndpoint.generated.h"

USTRUCT()
struct FDiscoveredPingEndpoint
{
	GENERATED_BODY()

	UPROPERTY()
	FString Region;

	UPROPERTY()
	FString Host;

	UPROPERTY()
	int32 Port = 0;
};
