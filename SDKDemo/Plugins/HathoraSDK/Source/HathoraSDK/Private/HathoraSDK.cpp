// Copyright 2023 Hathora, Inc.

#include "HathoraSDK.h"
#include "HathoraSDKModule.h"
#include "HathoraSDKConfig.h"
#include "HathoraSDKAuthV1.h"
#include "HathoraSDKDiscoveryV1.h"
#include "HathoraSDKLobbyV3.h"
#include "HathoraSDKProcessesV2.h"
#include "HathoraSDKRoomV2.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Forking/HathoraForkingSubsystem.h"

void UHathoraSDK::GetRegionalPings(const FHathoraOnGetRegionalPings& OnComplete, int32 NumPingsPerRegion)
{
	UHathoraSDK* SDK = UHathoraSDK::CreateHathoraSDK();
	SDK->AddToRoot(); // make sure this doesn't get garbage collected
	SDK->DiscoveryV1->GetRegionalPings(
		FHathoraOnGetRegionalPings::CreateLambda(
			[OnComplete, SDK](const FHathoraRegionPings& Result)
			{
				OnComplete.ExecuteIfBound(Result);
				SDK->RemoveFromRoot(); // Allow this SDK reference to be garbage collected
			}
		),
		NumPingsPerRegion
	);
}

UHathoraSDK* UHathoraSDK::CreateHathoraSDK()
{
	UHathoraSDK* SDK = NewObject<UHathoraSDK>();
	SDK->AuthV1 = NewObject<UHathoraSDKAuthV1>();
	SDK->DiscoveryV1 = NewObject<UHathoraSDKDiscoveryV1>();
	SDK->LobbyV3 = NewObject<UHathoraSDKLobbyV3>();
	SDK->ProcessesV2 = NewObject<UHathoraSDKProcessesV2>();
	SDK->RoomV2 = NewObject<UHathoraSDKRoomV2>();

	const UHathoraSDKConfig* Config = GetDefault<UHathoraSDKConfig>();
	FString AppId = Config->GetAppId();
	FHathoraSDKSecurity Security(Config->GetDevToken());

	if (Config->GetDevToken().Len() == 0)
	{
		UE_LOG(LogHathoraSDK, Warning, TEXT("No DevToken specified in Game.ini. This is required for server builds."));
	}

	SDK->SetCredentials(Config->GetAppId(), Security);

	return SDK;
}

FHathoraServerEnvironment UHathoraSDK::GetServerEnvironment()
{
	FHathoraServerEnvironment Environment;

	Environment.AppId = FPlatformMisc::GetEnvironmentVariable(TEXT("HATHORA_APP_ID"));
	Environment.AppSecret = FPlatformMisc::GetEnvironmentVariable(TEXT("HATHORA_APP_SECRET"));
	Environment.ProcessId = FPlatformMisc::GetEnvironmentVariable(TEXT("HATHORA_PROCESS_ID"));
	Environment.Region = ParseRegion(FPlatformMisc::GetEnvironmentVariable(TEXT("HATHORA_REGION")));
	Environment.RoomsPerProcess = FCString::Atoi(*FPlatformMisc::GetEnvironmentVariable(TEXT("HATHORA_ROOMS_PER_PROCESS")));

	return Environment;
}

FString UHathoraSDK::GetRegionString(EHathoraCloudRegion Region)
{
	FString RegionString = UEnum::GetValueAsString(Region);
	RegionString = RegionString.RightChop(RegionString.Find("::") + 2);

	return RegionString;
}

EHathoraCloudRegion UHathoraSDK::ParseRegion(FString RegionString)
{
	EHathoraCloudRegion Region = EHathoraCloudRegion::Unknown;
	for (uint32 i = 0; i < static_cast<uint8>(EHathoraCloudRegion::Unknown); i++)
	{
		FString CurrentRegion = UHathoraSDK::GetRegionString(static_cast<EHathoraCloudRegion>(i));
		if (CurrentRegion == RegionString)
		{
			Region = static_cast<EHathoraCloudRegion>(i);
			break;
		}
	}

	if (Region == EHathoraCloudRegion::Unknown)
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("[ParseRegion] Unknown region: %s"), *RegionString);
	}

	return Region;
}

FString UHathoraSDK::ParseErrorMessage(FString Content)
{
	TSharedPtr<FJsonObject> OutObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
	FJsonSerializer::Deserialize(Reader, OutObject);

	if (OutObject.IsValid())
	{
		FString Message;
		bool bValid = OutObject->TryGetStringField(TEXT("message"), Message);

		if (bValid)
		{
			return Message;
		}
		else
		{
			return Content;
		}
	}
	else
	{
		return Content;
	}
}

void UHathoraSDK::GetServerRoomId(float PollingInterval, FOnGetRoomId OnComplete)
{
	UHathoraSDK* SDK = UHathoraSDK::CreateHathoraSDK();
	SDK->AddToRoot(); // make sure this doesn't get garbage collected
	SDK->QueryServerRoomId(
		PollingInterval,
		FOnGetRoomId::CreateLambda(
			[OnComplete, SDK](const FString Result)
			{
				OnComplete.ExecuteIfBound(Result);
				SDK->RemoveFromRoot(); // Allow this SDK reference to be garbage collected
			}
		)
	);
}

void UHathoraSDK::QueryServerRoomId(float PollingInterval, FOnGetRoomId OnComplete)
{
	UWorld *World = GWorld;

	if (IsValid(World) && World->GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		if (UHathoraSDK::IsUsingBuiltInForking())
		{
			if (World->GetGameInstance() != nullptr)
			{
				UHathoraForkingSubsystem* ForkingSubsystem = World->GetGameInstance()->GetSubsystem<UHathoraForkingSubsystem>();
				if (ForkingSubsystem != nullptr)
				{
					FString RoomId = ForkingSubsystem->GetRoomId();

					if (RoomId == TEXT(""))
					{
						AsyncTask(ENamedThreads::AnyThread, [PollingInterval, OnComplete, this]()
						{
							FPlatformProcess::Sleep(PollingInterval);
							AsyncTask(ENamedThreads::GameThread, [PollingInterval, OnComplete, this]()
							{
								QueryServerRoomId(PollingInterval, OnComplete);
							});
						});
					}
					else
					{
						OnComplete.ExecuteIfBound(RoomId);
					}
				}
			}
			else
			{
				UE_LOG(LogHathoraSDK, Error, TEXT("ERROR: GameInstance is null; could not get the Room ID."));
				OnComplete.ExecuteIfBound(TEXT(""));
			}
		}
		else
		{
			FHathoraServerEnvironment HathoraEnvVars = UHathoraSDK::GetServerEnvironment();

			RoomV2->GetActiveRoomsForProcess(
				HathoraEnvVars.ProcessId,
				UHathoraSDKRoomV2::FHathoraOnGetRoomsForProcess::CreateLambda(
					[HathoraEnvVars, PollingInterval, OnComplete, this](const FHathoraGetRoomsForProcessResult& Result)
					{
						FString RoomId;

						if (Result.ErrorMessage.IsEmpty())
						{
							if (!Result.Data.IsEmpty())
							{
								RoomId = Result.Data[0].RoomId;
							}
						}
						else
						{
							UE_LOG(LogHathoraSDK, Error, TEXT("ERROR: Could not get active rooms for process id %s: %s"), *HathoraEnvVars.ProcessId, *Result.ErrorMessage);
						}

						if (RoomId == TEXT(""))
						{
							AsyncTask(ENamedThreads::AnyThread, [PollingInterval, OnComplete, this]()
							{
								FPlatformProcess::Sleep(PollingInterval);
								AsyncTask(ENamedThreads::GameThread, [PollingInterval, OnComplete, this]()
								{
									QueryServerRoomId(PollingInterval, OnComplete);
								});
							});
						}
						else
						{
							OnComplete.ExecuteIfBound(RoomId);
						}
					}
				)
			);
		}
	}
	else
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("Called QueryServerRoomId from a client or the world is not valid."));
		OnComplete.ExecuteIfBound(TEXT(""));
	}
}

bool UHathoraSDK::IsUsingBuiltInForking()
{
	const UHathoraSDKConfig* Config = GetDefault<UHathoraSDKConfig>();
	return Config->GetUseBuiltInForking();
}

void UHathoraSDK::SetAuthToken(FString Token)
{
	const UHathoraSDKConfig* Config = GetDefault<UHathoraSDKConfig>();
	FString AppId = Config->GetAppId();
	FHathoraSDKSecurity Security(Token);

	SetCredentials(Config->GetAppId(), Security);
}

bool UHathoraSDK::IsLoggedIn()
{
	return AuthV1->IsLoggedIn();
}

void UHathoraSDK::SetCredentials(FString AppId, FHathoraSDKSecurity Security)
{
	AuthV1->SetCredentials(AppId, Security);
	DiscoveryV1->SetCredentials(AppId, Security);
	LobbyV3->SetCredentials(AppId, Security);
	ProcessesV2->SetCredentials(AppId, Security);
	RoomV2->SetCredentials(AppId, Security);
}
