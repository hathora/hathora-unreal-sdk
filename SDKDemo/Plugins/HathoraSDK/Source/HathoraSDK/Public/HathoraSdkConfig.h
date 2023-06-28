// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HathoraSdkConfig.generated.h"

UCLASS(Config = Game)
class HATHORASDK_API UHathoraSdkConfig : public UObject
{
	GENERATED_BODY()

public:
	UHathoraSdkConfig();
	const FString& GetBaseUrl() const { return BaseUrl; };
	int32		   GetPingMeasurementsToTake() const { return PingMeasurementsToTake; }

private:
	FString BaseUrl;
	int32	PingMeasurementsToTake = 10;
};
