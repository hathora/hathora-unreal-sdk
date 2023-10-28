// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "HathoraTypes.generated.h"

USTRUCT(BlueprintType)
struct FHathoraDiscoveredPingEndpoint
{
	GENERATED_BODY()

	// The name of the Hathora Cloud region (e.g. "Seattle").
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString Region;

	// The pingable host of the Hathora Cloud region. The host for
	// a specific region may change from time to time. You can use
	// this host for both ICMP and WebSocket pings.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString Host;

	// The port to use if you're using the WebSocket echo server to
	// get ping results.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	int32 Port = 0;
};

USTRUCT(BlueprintType)
struct FHathoraRegionPings
{
	GENERATED_BODY()

	// The ping times in milliseconds for each region. The key is the name of the region.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	TMap<FString, int32> Pings;
};

USTRUCT(BlueprintType)
struct FHathoraSDKSecurity
{
	GENERATED_BODY()

	// The HathoraDevToken used for the Authorization HTTP header.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	FString HathoraDevToken;
};

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(FHathoraOnGetRegionalPings, FHathoraRegionPings, Result);

UENUM(BlueprintType)
enum class EHathoraCloudRegion : uint8
{
	Seattle,
	Washington_DC,
	Chicago,
	London,
	Frankfurt,
	Mumbai,
	Singapore,
	Tokyo,
	Sydney,
	Sao_Paulo
};
