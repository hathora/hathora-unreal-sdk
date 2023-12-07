// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "HathoraSDKLobbyV3.h"
#include "DemoMatchGameState.generated.h"

class UHathoraSDK;

UCLASS(BlueprintType, Blueprintable)
class ADemoMatchGameState : public AGameStateBase
{
	GENERATED_UCLASS_BODY()
public:
	//~ Begin UActorComponent Interface.
	virtual void BeginPlay() override;
	//~ End UActorComponent Interface.

	UPROPERTY(Replicated)
	FHathoraLobbyInfo LobbyInfo;

	UPROPERTY(Replicated)
	bool bLobbyIsReady = false;

	UPROPERTY(ReplicatedUsing = OnRep_MatchTime)
	int32 MatchTime = -120;

	UDELEGATE()
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDemoMatchLobbyReady);

	UPROPERTY(BlueprintAssignable)
	FDemoMatchLobbyReady OnLobbyReady;

	UDELEGATE()
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDemoMatchTimeUpdated, int32, MatchTime);

	UPROPERTY(BlueprintAssignable)
	FDemoMatchTimeUpdated OnMatchTimeUpdated;

private:
	UHathoraSDK* SDK;
	FHathoraServerEnvironment HathoraEnvVars;
	FString RoomId;

	void QueryActiveRooms();

	void ProcessMatchTime();

	UFUNCTION(NetMulticast, Reliable)
	void MC_LobbyReady();

	UFUNCTION()
	void OnRep_MatchTime();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
