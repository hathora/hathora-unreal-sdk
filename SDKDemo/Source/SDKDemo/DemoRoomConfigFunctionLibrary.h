// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "JsonObjectConverter.h"
#include "HathoraSDK.h"
#include "DemoRoomConfigFunctionLibrary.generated.h"

USTRUCT(BlueprintType)
struct FDemoRoomConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Config")
	FString RoomName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Config")
	bool bMatchStarted = false;
};

UCLASS(BlueprintType)
class UDemoRoomConfigFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Room Config")
	static FString SerializeRoomConfigToString(const FDemoRoomConfig& RoomConfig)
	{
		FString OutString;
		bool bResult = FJsonObjectConverter::UStructToJsonObjectString(RoomConfig, OutString);

		if (!bResult)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to serialize room config to string"));
		}

#if WITH_SERVER_CODE && !WITH_EDITOR
		OutString = UHathoraSDK::AddPortNameToRoomConfig(OutString);
#endif

		return OutString;
	}

	UFUNCTION(BlueprintCallable, Category = "Room Config")
	static FDemoRoomConfig DeserializeRoomConfigFromString(const FString& JsonString)
	{
		FString JsonStringWithoutPort = UHathoraSDK::RemovePortNameFromRoomConfig(JsonString);

		FDemoRoomConfig OutRoomConfig;
		bool bResult = FJsonObjectConverter::JsonObjectStringToUStruct(JsonStringWithoutPort, &OutRoomConfig, 0, 0);

		if (!bResult)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to deserialize JSON string to room config"));
		}

		return OutRoomConfig;
	}
};
