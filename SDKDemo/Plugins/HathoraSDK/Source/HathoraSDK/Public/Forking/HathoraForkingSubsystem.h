// Copyright 2024 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TimerHandle.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "HathoraForkingSubsystem.generated.h"

class UHathoraSDK;

// This class is a GameInstanceSubsystem that handles forking a parent process
// into several child processes, each with its own port. Read any associated
// Hathora online documentation, the ./README.md file, and the various comments
// documented throughout ../../Private/Forking/HathoraForkingSubsystem.cpp for
// more information on how to use this functionality.
UCLASS(BlueprintType)
class HATHORASDK_API UHathoraForkingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Begin USubsystem
	virtual void Initialize(FSubsystemCollectionBase &Collection) override;
	virtual void Deinitialize() override;
	// End USubsystem

	void Fork();

	static const FString RoomConfigPortKey;
	static const FString DefaultPortName;
	static const FString ExposedPortNamePrefix;
	static const float RoomAllocationPollingInterval;

	static int32 GetPortFromRoomConfig(const FString &RoomConfig);
	static FString RemovePortFromRoomConfig(const FString &RoomConfig);

	FString AddPortToRoomConfig(const FString &RoomConfig);

	UFUNCTION(BlueprintPure, Category = "HathoraSDK|Forking")
	FString GetRoomId() const { return RoomId; }

private:
	int32 StartingPort;
	FString RoomId;
	int32 ChildIdx;

	TArray<int32> ExposedPorts;

	UPROPERTY()
	UHathoraSDK* SDK;

	void GetExposedPorts();

	void WaitForRoomAllocation();
};
