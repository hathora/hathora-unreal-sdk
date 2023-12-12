// Copyright 2023 Hathora, Inc.

#include "HathoraLobbyComponent.h"
#include "HathoraSDKModule.h"
#include "HathoraSDK.h"
#include "HathoraSDKAuthV1.h"

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
						JoinRoomIdWhenReady = Result.Data.RoomId;
						GetLobbyConnectionInfo(JoinRoomIdWhenReady);
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

void UHathoraLobbyComponent::JoinLobbyWithConnectionInfo(FHathoraConnectionInfo ConnectionInfo)
{
	if (GEngine)
	{
		int32 Port = ConnectionInfo.ExposedPort.Port;
		FNumberFormattingOptions NumberFormatOptions;
		NumberFormatOptions.UseGrouping = false;
		FString PortString = FText::AsNumber(Port, &NumberFormatOptions).ToString();
		FString JoinCommand = FString::Printf(TEXT("open %s:%s"), *ConnectionInfo.ExposedPort.Host, *PortString);
		GEngine->Exec(GetWorld(), *JoinCommand);
	}
}

void UHathoraLobbyComponent::GetLobbyConnectionInfo(FString RoomId)
{
	if (!IsValid(SDK) || !IsValid(SDK->RoomV2))
	{
		FString ErrorMessage = TEXT("Get lobby connection info failed: Hathora SDK is not valid");
		UE_LOG(LogHathoraSDK, Error, TEXT("%s"), *ErrorMessage);
		OnError.Broadcast(ErrorMessage);
		return;
	}

	SDK->RoomV2->GetConnectionInfo(
		RoomId,
		UHathoraSDKRoomV2::FHathoraOnRoomConnectionInfo::CreateLambda(
			[this, RoomId](const FHathoraRoomConnectionInfoResult& Result)
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
						if (Result.Data.RoomId == JoinRoomIdWhenReady)
						{
							JoinLobbyWithConnectionInfo(Result.Data);
						}
						else
						{
							PreReadyLobbies.Add(Result.Data.RoomId, Result.Data);
							GetPublicLobbyInfo(Result.Data.RoomId);
						}
					}
					else
					{
						// The lobby is still being created, so we need to wait for it to be ready.
						UWorld* World = GetWorld();
						if (IsValid(World))
						{
							FTimerHandle TimerHandle;
							World->GetTimerManager().SetTimer(
								TimerHandle,
								[this, RoomId]()
								{
									GetLobbyConnectionInfo(RoomId);
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

void UHathoraLobbyComponent::GetPublicLobbyInfo(FString RoomId)
{
	if (!IsValid(SDK) || !IsValid(SDK->LobbyV3))
	{
		FString ErrorMessage = TEXT("Get public lobby info failed: Hathora SDK is not valid");
		UE_LOG(LogHathoraSDK, Error, TEXT("%s"), *ErrorMessage);
		OnError.Broadcast(ErrorMessage);
		return;
	}

	SDK->LobbyV3->GetLobbyInfoByRoomId(
		RoomId,
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
					FHathoraConnectionInfo* Value = PreReadyLobbies.Find(Result.Data.RoomId);
					if (Value != nullptr)
					{
						OnLobbyReady.Broadcast(*Value, Result.Data.RoomConfig, Result.Data.Region);
					}
					else
					{
						FString ErrorMessage = FString::Printf(TEXT("Could not find the connection info for public lobby (%s) after we fetched for it."), *Result.ErrorMessage);
						UE_LOG(LogHathoraSDK, Error, TEXT("%s"), *ErrorMessage);
						OnError.Broadcast(ErrorMessage);
					}
				}
				else
				{
					FString ErrorMessage = FString::Printf(TEXT("Get Lobby Info for public lobby failed: %s"), *Result.ErrorMessage);
					UE_LOG(LogHathoraSDK, Error, TEXT("%s"), *ErrorMessage);
					OnError.Broadcast(ErrorMessage);
				}
			}
		)
	);
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
						GetLobbyConnectionInfo(LobbyInfo.RoomId);
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
					JoinRoomIdWhenReady = Result.Data.RoomId;
					GetLobbyConnectionInfo(JoinRoomIdWhenReady);
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

