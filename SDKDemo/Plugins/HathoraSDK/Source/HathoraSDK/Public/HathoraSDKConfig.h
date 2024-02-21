// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/DeveloperSettings.h"

#include "HathoraSDKConfig.generated.h"

UCLASS(Config = Game, meta = (DisplayName = "Hathora SDK"))
class HATHORASDK_API UHathoraSDKConfig : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UHathoraSDKConfig()
	{
		CategoryName = TEXT("Plugins");
		SectionName = TEXT("HathoraSDK");
	}

	const FString& GetBaseUrl() const { return BaseUrl; };

	const float& GetPingTimeoutSeconds() const { return PingTimeoutSeconds; };

	const FString& GetAppId() const { return AppId; };

	const FString& GetDevToken() const { return DevToken; };

	const bool GetUseBuiltInForking() const { return bUseBuiltInForking; };

private:
	UPROPERTY(Config, EditAnywhere, Category = "Network")
	FString BaseUrl = "https://api.hathora.dev";

	UPROPERTY(Config, EditAnywhere, Category = "Network")
	float PingTimeoutSeconds = 1.0f;

	// Hathora Cloud AppId
	UPROPERTY(Config, EditAnywhere, Category = "Authentication")
	FString AppId;

	// The HathoraDevToken to use in packaged server builds.
	// Don't save this as part of your Config/DefaultGame.ini file,
	// Use Config/DedicatedServerGame.ini instead.
	UPROPERTY(Config, EditAnywhere, Category = "Authentication")
	FString DevToken;

	// Use the built-in mechanisms for handling forking to support
	// multiple rooms per process. This is disabled by default,
	// and you should read the documentation if you're interested in
	// using it. If you are implementing your own forking mechanism or
	// using another plugin that handles forking, keep this flag disabled.
	// This should be considered EXPERIMENTAL.
	UPROPERTY(Config, EditAnywhere, Category = "Processes")
	bool bUseBuiltInForking = false;
};
