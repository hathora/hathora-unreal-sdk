// Copyright 2024 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TimerHandle.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "HathoraGameInstanceSubsystem.generated.h"

class UHathoraSDK;

UCLASS(BlueprintType)
class HATHORASDK_API UHathoraGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Begin USubsystem
	virtual void Initialize(FSubsystemCollectionBase &Collection) override;
	virtual void Deinitialize() override;
	// End USubsystem

	UFUNCTION(BlueprintPure, Category = "HathoraSDK")
	FString GetRoomId() const { return RoomId; }

private:
	FString RoomId;
	int32 ChildIdx;

	UPROPERTY()
	UHathoraSDK* SDK;

	void WaitForRoomAllocation();
};
