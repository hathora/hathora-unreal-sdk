// Copyright 2023 Hathora, Inc.

#include "DemoMatchGameState.h"
#include "HathoraSDK.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "HathoraSDKRoomV2.h"
#include "HathoraSDKLobbyV3.h"
#include "../DemoRoomConfigFunctionLibrary.h"

ADemoMatchGameState::ADemoMatchGameState(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;
}

void ADemoMatchGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADemoMatchGameState, LobbyInfo);
	DOREPLIFETIME(ADemoMatchGameState, bLobbyIsReady);
	DOREPLIFETIME(ADemoMatchGameState, MatchTime);
}

void ADemoMatchGameState::OnRep_MatchTime()
{
	OnMatchTimeUpdated.Broadcast(MatchTime);
}

void ADemoMatchGameState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SDK = UHathoraSDK::CreateHathoraSDK();
		check(SDK);

		HathoraEnvVars = UHathoraSDK::GetServerEnvironment();

		QueryActiveRooms();
	}
}

void ADemoMatchGameState::QueryActiveRooms()
{
	SDK->RoomV2->GetActiveRoomsForProcess(
		HathoraEnvVars.ProcessId,
		UHathoraSDKRoomV2::FHathoraOnGetRoomsForProcess::CreateLambda(
			[this](const FHathoraGetRoomsForProcessResult& Result)
			{
				if (Result.ErrorMessage.IsEmpty())
				{
					if (!Result.Data.IsEmpty())
					{
						// NOTE: THIS ASSUMES ONLY 1 ROOM PER PROCESS
						// YOU WILL NEED TO CHANGE THIS LOGIC OTHERWISE

						RoomId = Result.Data[0].RoomId;

						SDK->LobbyV3->GetLobbyInfoByRoomId(
							RoomId,
							UHathoraSDKLobbyV3::FHathoraOnLobbyInfo::CreateLambda(
								[this](const FHathoraLobbyInfoResult& LobbyInfoResult)
								{
									if (LobbyInfoResult.ErrorMessage.IsEmpty())
									{
										LobbyInfo = LobbyInfoResult.Data;
										bLobbyIsReady = true;

										// Provide a short to allow variables to replicate first
										FTimerHandle TimerHandle;
										GetWorld()->GetTimerManager().SetTimer(
											TimerHandle,
											[this]()
											{
												MC_LobbyReady();
												ProcessMatchTime();
											},
											1.0f,
											false
										);
									}
									else
									{
										UE_LOG(LogTemp, Error, TEXT("ERROR: Could not get lobby info for room id %s: %s"), *RoomId, *LobbyInfoResult.ErrorMessage);
									}
								}
							)
						);
					}
					else
					{
						// While this likely shouldn't happen, let's try again
						FTimerHandle TimerHandle;
						GetWorld()->GetTimerManager().SetTimer(
							TimerHandle,
							[this]()
							{
								QueryActiveRooms();
							},
							1.0f,
							false
						);
					}
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("ERROR: Could not get active rooms for process id %s: %s"), *HathoraEnvVars.ProcessId, *Result.ErrorMessage);
				}
			}
		)
	);
}

void ADemoMatchGameState::MC_LobbyReady_Implementation()
{
	OnLobbyReady.Broadcast();
}

void ADemoMatchGameState::ProcessMatchTime()
{
	MatchTime++;

	if (MatchTime == 0)
	{
		FDemoRoomConfig RoomConfig = UDemoRoomConfigFunctionLibrary::DeserializeRoomConfigFromString(LobbyInfo.RoomConfig);
		RoomConfig.bMatchStarted = true;
		FString NewRoomConfig = UDemoRoomConfigFunctionLibrary::SerializeRoomConfigToString(RoomConfig, GetWorld());

		// Tell Hathora that the match has started, so
		// new clients won't see the match in the lobby list
		// (specific to this demo implementation)
		SDK->RoomV2->UpdateRoomConfig(
			RoomId,
			NewRoomConfig,
			UHathoraSDKRoomV2::FHathoraOnUpdateRoomConfig::CreateLambda(
				[this](const FHathoraUpdateRoomConfigResult& Result)
				{
					// no need to process the result
				}
			)
		);
	}
	else if (MatchTime >= 120)
	{
		// Kill the match after 2 minutes
		UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
		return;
	}

	// Process match time every second
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this]()
		{
			ProcessMatchTime();
		},
		1.0f,
		false
	);
}
