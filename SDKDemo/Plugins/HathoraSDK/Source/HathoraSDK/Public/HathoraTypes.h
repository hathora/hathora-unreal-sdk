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
	GENERATED_USTRUCT_BODY()

	FHathoraSDKSecurity() {}

	FHathoraSDKSecurity(FString InAuthToken)
	{
		this->AuthToken = InAuthToken;
	}

	// The token to use in the Authorization HTTP header in API calls.
	// This can be either the HathoraDevToken for server/trusted calls,
	// or the player auth token for client/untrusted calls.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	FString AuthToken;
};

typedef TDelegate<void(const FHathoraRegionPings&)> FHathoraOnGetRegionalPings;

UENUM(BlueprintType)
enum class EHathoraCloudRegion : uint8
{
	Seattle,
	Los_Angeles,
	Washington_DC,
	Chicago,
	Dallas,
	London,
	Frankfurt,
	Mumbai,
	Singapore,
	Tokyo,
	Sydney,
	Sao_Paulo,
	Dubai,
	Johannesburg,
	Unknown UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FHathoraExposedPort
{
	GENERATED_BODY()

	// Transport type specifies the underlying communication
	// protocol to the exposed port. "tcp", "udp", or "tls".
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString TransportType;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	int32 Port = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString Host;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString Name;
};

USTRUCT(BlueprintType)
struct FHathoraServerEnvironment
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString AppId;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString AppSecret;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString ProcessId;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString DeploymentId;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString BuildTag;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	EHathoraCloudRegion Region = EHathoraCloudRegion::Unknown;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	int32 RoomsPerProcess = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString InitialRoomId;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString InitialRoomConfig;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString Hostname;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	int32 DefaultPort = 0;
};
