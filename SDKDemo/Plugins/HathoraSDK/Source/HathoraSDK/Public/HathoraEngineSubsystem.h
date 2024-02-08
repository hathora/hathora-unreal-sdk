// Copyright 2024 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TimerHandle.h"
#include "Subsystems/EngineSubsystem.h"

#include "HathoraEngineSubsystem.generated.h"

UCLASS(BlueprintType)
class HATHORASDK_API UHathoraEngineSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	// Begin USubsystem
	virtual void Initialize(FSubsystemCollectionBase &Collection) override;
	virtual void Deinitialize() override;
	// End USubsystem

	UFUNCTION(BlueprintCallable, Category = "Hathora")
	int32 GetPortFromRoomConfig(const FString &RoomConfig);

	UFUNCTION(BlueprintCallable, Category = "Hathora")
	FString AddPortToRoomConfig(const FString &RoomConfig);

	UFUNCTION(BlueprintCallable, Category = "Hathora")
	FString RemovePortFromRoomConfig(const FString &RoomConfig);

private:
	int32 Port;
	FString WaitForRoomAllocation();
};
