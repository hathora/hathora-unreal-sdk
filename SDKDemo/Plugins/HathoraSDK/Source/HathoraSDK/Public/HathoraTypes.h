// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "HathoraTypes.generated.h"

USTRUCT(BlueprintType)
struct FHathoraDiscoveredPingEndpoint
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString Region;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString Host;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	int32 Port = 0;
};

USTRUCT(BlueprintType)
struct FHathoraRegionPings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	TMap<FString, int32> Pings;
};

USTRUCT(BlueprintType)
struct FHathoraSDKSecurity
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	FString HathoraDevToken;
};

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(FHathoraOnGetRegionalPings, FHathoraRegionPings, Result);
