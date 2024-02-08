// Copyright 2023 Hathora, Inc.

#include "HathoraLobbyComponent.h"
#include "HathoraSDKModule.h"
#include "HathoraSDK.h"
#include "HathoraSDKAuthV1.h"
#include "HathoraSDKLobbyV3.h"
#include "HathoraSDKRoomV2.h"
#include "HathoraEngineSubsystem.h"

void UHathoraLobbyComponent::BeginPlay()
{
	Super::BeginPlay();

	SDK = UHathoraSDK::CreateHathoraSDK();
}

bool UHathoraLobbyComponent::IsLoggedIn()
{
	return IsValid(SDK) && SDK->IsLoggedIn();
}

void UHathoraLobbyComponent::CreateAndJoinLobby(
	EHathoraLobbyVisibility Visibility,
	FString RoomConfig,
	EHathoraCloudRegion Region,
	int32 ShortCodeLength
) {
	FString ShortCode = FGuid::NewGuid().ToString().Left(ShortCodeLength);
	CreateAndJoinLobbyCustomShortCode(Visibility, RoomConfig, Region, ShortCode);
}

void UHathoraLobbyComponent::CreateAndJoinLobbyCustomShortCode(
	EHathoraLobbyVisibility Visibility,
	FString RoomConfig,
	EHathoraCloudRegion Region,
	FString ShortCode
) {
	if (IsLoggedIn())
	{
		SDK->LobbyV3->CreateLobby(
			Visibility,
			RoomConfig,
			Region,
			ShortCode,
			TEXT(""),
			UHathoraSDKLobbyV3::FHathoraOnLobbyInfo::CreateLambda(
				[this, RoomConfig](const FHathoraLobbyInfoResult& Result)
				{
					if (!IsValid(this))
					{
						// this object is being destroyed, so let's not process anything
						FString ErrorMessage = FString::Printf(TEXT("Create Lobby failed: Lobby Component is no longer valid"));
						UE_LOG(LogHathoraSDK, Error, TEXT("%s"), *ErrorMessage);
						return;
					}

					if (Result.ErrorMessage.IsEmpty())
					{
						GetLobbyConnectionInfo(
							Result.Data.RoomId,
							FHathoraOnGetGetLobbyConnectionInfo::CreateLambda(
								[this](const FHathoraConnectionInfo& ConnectionInfo)
								{
									JoinLobbyWithConnectionInfo(ConnectionInfo);
								}
							)
						);
					}
					else
					{
						FString ErrorMessage = FString::Printf(TEXT("Create Lobby failed: %s"), *Result.ErrorMessage);
						UE_LOG(LogHathoraSDK, Error, TEXT("%s"), *ErrorMessage);
						OnError.Broadcast(ErrorMessage);
					}
				}
			)
		);
	}
	else
	{
		FString ErrorMessage = TEXT("Create Lobby failed: Player is not logged in");
		UE_LOG(LogHathoraSDK, Error, TEXT("%s"), *ErrorMessage);
		OnError.Broadcast(ErrorMessage);
	}
}

void UHathoraLobbyComponent::FetchPublicLobbies()
{
	if (!IsValid(SDK) || !IsValid(SDK->LobbyV3))
	{
		FString ErrorMessage = TEXT("Fetch public lobbies failed: Hathora SDK is not valid");
		UE_LOG(LogHathoraSDK, Error, TEXT("%s"), *ErrorMessage);
		OnError.Broadcast(ErrorMessage);
		return;
	}

	SDK->LobbyV3->ListAllActivePublicLobbies(
		UHathoraSDKLobbyV3::FHathoraOnLobbyInfos::CreateLambda(
			[this](const FHathoraLobbyInfosResult& Result)
			{
				if (!IsValid(this))
				{
					// this object is being destroyed, so let's not process anything
					FString ErrorMessage = FString::Printf(TEXT("Create Lobby failed: Lobby Component is no longer valid"));
					UE_LOG(LogHathoraSDK, Error, TEXT("%s"), *ErrorMessage);
					return;
				}

				if (Result.ErrorMessage.IsEmpty())
				{
					for (FHathoraLobbyInfo LobbyInfo : Result.Data)
					{
						GetLobbyConnectionInfo(
							LobbyInfo.RoomId,
							FHathoraOnGetGetLobbyConnectionInfo::CreateLambda(
								[this, LobbyInfo](const FHathoraConnectionInfo& ConnectionInfo)
								{
									OnLobbyReady.Broadcast(ConnectionInfo, LobbyInfo.RoomConfig, LobbyInfo.Region);
								}
							)
						);
					}
				}
				else
				{
					FString ErrorMessage = FString::Printf(TEXT("Fetch public lobbies failed: %s"), *Result.ErrorMessage);
					UE_LOG(LogHathoraSDK, Error, TEXT("%s"), *ErrorMessage);
					OnError.Broadcast(ErrorMessage);
				}
			}
		)
	);
}

void UHathoraLobbyComponent::JoinLobbyWithShortCode(FString ShortCode)
{
	if (!IsValid(SDK) || !IsValid(SDK->LobbyV3))
	{
		FString ErrorMessage = TEXT("Join lobby with short code failed: Hathora SDK is not valid");
		UE_LOG(LogHathoraSDK, Error, TEXT("%s"), *ErrorMessage);
		OnError.Broadcast(ErrorMessage);
		return;
	}

	SDK->LobbyV3->GetLobbyInfoByShortCode(
		ShortCode,
		UHathoraSDKLobbyV3::FHathoraOnLobbyInfo::CreateLambda(
			[this](const FHathoraLobbyInfoResult& Result)
			{
				if (!IsValid(this))
				{
					// this object is being destroyed, so let's not process anything
					FString ErrorMessage = FString::Printf(TEXT("Create Lobby failed: Lobby Component is no longer valid"));
					UE_LOG(LogHathoraSDK, Error, TEXT("%s"), *ErrorMessage);
					return;
				}

				if (Result.ErrorMessage.IsEmpty())
				{
					GetLobbyConnectionInfo(
						Result.Data.RoomId,
						FHathoraOnGetGetLobbyConnectionInfo::CreateLambda(
							[this](const FHathoraConnectionInfo& ConnectionInfo)
							{
								JoinLobbyWithConnectionInfo(ConnectionInfo);
							}
						)
					);
				}
				else
				{
					FString ErrorMessage = FString::Printf(TEXT("Join lobby with short code failed: %s"), *Result.ErrorMessage);
					UE_LOG(LogHathoraSDK, Error, TEXT("%s"), *ErrorMessage);
					OnError.Broadcast(ErrorMessage);
				}
			}
		)
	);
}

void UHathoraLobbyComponent::JoinLobbyWithConnectionInfo(FHathoraConnectionInfo ConnectionInfo)
{
	if (GEngine)
	{
		FNumberFormattingOptions NumberFormatOptions;
		NumberFormatOptions.UseGrouping = false;
		FString PortString = FText::AsNumber(ConnectionInfo.ExposedPort.Port, &NumberFormatOptions).ToString();
		FString JoinCommand = FString::Printf(TEXT("open %s:%s"), *ConnectionInfo.ExposedPort.Host, *PortString);
		GEngine->Exec(GetWorld(), *JoinCommand);
	}
}

void UHathoraLobbyComponent::GetLobbyConnectionInfo(FString RoomId, FHathoraOnGetGetLobbyConnectionInfo OnResult)
{
	if (!IsValid(SDK) || !IsValid(SDK->RoomV2))
	{
		FString ErrorMessage = TEXT("Get lobby connection info failed: Hathora SDK is not valid");
		UE_LOG(LogHathoraSDK, Error, TEXT("%s"), *ErrorMessage);
		OnError.Broadcast(ErrorMessage);
		return;
	}

	SDK->RoomV2->GetRoomInfo(
		RoomId,
		UHathoraSDKRoomV2::FHathoraOnGetRoomInfo::CreateLambda(
			[this, RoomId, OnResult](const FHathoraGetRoomInfoResult& Result)
			{
				if (!IsValid(this))
				{
					// this object is being destroyed, so let's not process anything
					FString ErrorMessage = FString::Printf(TEXT("GetLobbyConnectionInfo failed: Lobby Component is no longer valid"));
					UE_LOG(LogHathoraSDK, Error, TEXT("%s"), *ErrorMessage);
					return;
				}

				if (Result.ErrorMessage.IsEmpty())
				{
					if (Result.Data.Status == EHathoraRoomStatus::Active)
					{
						UHathoraEngineSubsystem* HathoraEngineSubsystem = GEngine->GetEngineSubsystem<UHathoraEngineSubsystem>();
						if (HathoraEngineSubsystem)
						{
							int32 Port = HathoraEngineSubsystem->GetPortFromRoomConfig(Result.Data.RoomConfig);
							if (Port != 0)
							{
								GetLobbyConnectionInfo(RoomId, Port, OnResult);
								return;
							}
						}
						else
						{
							FString ErrorMessage = FString::Printf(TEXT("GetLobbyConnectionInfo failed: Hathora Engine Subsystem is not valid"));
							UE_LOG(LogHathoraSDK, Error, TEXT("%s"), *ErrorMessage);
						}
					}
				}

				// We need to wait for the room to be ready
				UWorld* World = GetWorld();
				if (IsValid(World))
				{
					FTimerHandle TimerHandle;
					World->GetTimerManager().SetTimer(
						TimerHandle,
						[this, RoomId, OnResult]()
						{
							GetLobbyConnectionInfo(RoomId, OnResult);
						},
						1.0f,
						false
					);
				}
			}
		)
	);
}

void UHathoraLobbyComponent::GetLobbyConnectionInfo(FString RoomId, int32 Port, FHathoraOnGetGetLobbyConnectionInfo OnResult)
{
	SDK->RoomV2->GetConnectionInfo(
		RoomId,
		UHathoraSDKRoomV2::FHathoraOnRoomConnectionInfo::CreateLambda(
			[this, RoomId, Port, OnResult](const FHathoraRoomConnectionInfoResult& Result)
			{
				if (!IsValid(this))
				{
					// this object is being destroyed, so let's not process anything
					FString ErrorMessage = FString::Printf(TEXT("Create Lobby failed: Lobby Component is no longer valid"));
					UE_LOG(LogHathoraSDK, Error, TEXT("%s"), *ErrorMessage);
					return;
				}

				if (Result.ErrorMessage.IsEmpty())
				{
					if (Result.Data.Status == TEXT("active"))
					{
						// We ignore the Hathora-provided port as the server provides it in the roomConfig.
						// This assumes you have specified all of the potential ports in the Hathora additional
						// exposed ports to ensure proper port forwarding occurs.
						FHathoraConnectionInfo ConnectionInfo = Result.Data;
						ConnectionInfo.ExposedPort.Port = Port;
						OnResult.ExecuteIfBound(ConnectionInfo);
					}
					else
					{
						// The lobby is still being created, so we need to wait for it to be ready.
						// This should not really happen because we already checked for Active in
						// the previous GetLobbyConnectionInfo function, but this is a safety check.
						UWorld* World = GetWorld();
						if (IsValid(World))
						{
							FTimerHandle TimerHandle;
							World->GetTimerManager().SetTimer(
								TimerHandle,
								[this, RoomId, Port, OnResult]()
								{
									GetLobbyConnectionInfo(RoomId, Port, OnResult);
								},
								1.0f,
								false
							);
						}
					}
				}
				else
				{
					FString ErrorMessage = FString::Printf(TEXT("Get lobby connection info failed: %s"), *Result.ErrorMessage);
					UE_LOG(LogHathoraSDK, Error, TEXT("%s"), *ErrorMessage);
					OnError.Broadcast(ErrorMessage);
				}
			}
		)
	);
}
