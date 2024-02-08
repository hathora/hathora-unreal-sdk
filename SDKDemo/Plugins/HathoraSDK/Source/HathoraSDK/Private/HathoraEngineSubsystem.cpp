// Copyright 2024 Hathora, Inc.

#include "HathoraEngineSubsystem.h"
#include "HathoraSDKModule.h"
#include "HathoraSDK.h"
#include "HathoraTypes.h"
#include "HathoraForkProcess.h"
#include "HathoraSDKRoomV2.h"

#if PLATFORM_LINUX
#include <fcntl.h>
#include <semaphore.h>
#endif

void UHathoraEngineSubsystem::Initialize(FSubsystemCollectionBase &Collection)
{
	Super::Initialize(Collection);

	UWorld *World = GetWorld();

	FURL DefaultUrl;
	Port = DefaultUrl.Port;

#if PLATFORM_LINUX
	if (IsValid(World) && World->GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		FHathoraServerEnvironment HathoraEnvVars = UHathoraSDK::GetServerEnvironment();

		if (HathoraEnvVars.RoomsPerProcess > 1) {
			FString BasePath = FPaths::Combine(
				FPaths::ProjectSavedDir(),
				TEXT("Hathora"),
				TEXT("RoomConsoleVars")
			);

			FCommandLine::Append(*FString(TEXT("-NumForks=") + FString::FromInt(HathoraEnvVars.RoomsPerProcess)));
			FCommandLine::Append(TEXT("-WaitAndFork"));
			FCommandLine::Append(*FString(TEXT("-WaitAndForkCmdLinePath=") + BasePath));

			for (int32 i = 0; i < HathoraEnvVars.RoomsPerProcess; i++)
			{
				FString RoomConsoleVarsPath = FPaths::Combine(
					BasePath,
					FString::Printf(TEXT("%d"), i + 1)
				);

				FFileHelper::SaveStringToFile(TEXT("-port=") + FString::FromInt(7777 + i), *RoomConsoleVarsPath);
			}

			// Create a named semaphore
			sem_t* sem = sem_open("/hathora/rooms/allocation", O_CREAT, 0644, 1);
			if (sem == SEM_FAILED)
			{
				UE_LOG(LogHathoraSDK, Error, TEXT("Failed to create semaphore"));
				FPlatformMisc::RequestExitWithStatus(true, 1);
				return;
			}

			FHathoraForkProcess::ForkIfRequested(World);

			// only children should get to this point
			FURL DefaultUrl2;
			UE_LOG(LogHathoraSDK, Log, TEXT("Forked process started with port: %d"), DefaultUrl2.Port);
			int32 ChildIdx = DefaultUrl2.Port - 7777;
			Port = DefaultUrl2.Port;

			if (sem_wait(sem) == -1)
			{
				UE_LOG(LogHathoraSDK, Error, TEXT("There was an error when trying to acquire the semaphore"));
				FPlatformMisc::RequestExitWithStatus(true, 1);
				return;
			}

			FString RoomId = WaitForRoomAllocation();

			if (sem_post(sem) == -1)
			{
				UE_LOG(LogHathoraSDK, Error, TEXT("There was an error when trying to release the semaphore"));
				FPlatformMisc::RequestExitWithStatus(true, 1);
				return;
			}
		}
		else
		{
			UHathoraSDK *SDK = UHathoraSDK::CreateHathoraSDK();
			SDK->RoomV2->GetActiveRoomsForProcess(
				HathoraEnvVars.ProcessId,
				UHathoraSDKRoomV2::FHathoraOnGetRoomsForProcess::CreateLambda(
					[SDK, this](const FHathoraGetRoomsForProcessResult &Result)
					{
						if (Result.StatusCode == 200 && Result.Data.Num() > 0)
						{
							FHathoraProcessRoomInfo Room = Result.Data[0];

							TSharedPtr<FJsonObject> OutRoomConfig;
							TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Room.RoomConfig);
							FJsonSerializer::Deserialize(Reader, OutRoomConfig);

							OutRoomConfig->SetNumberField(TEXT("hathoraGamePort"), Port);
							FString RoomConfigString;
							TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RoomConfigString);
							FJsonSerializer::Serialize(OutRoomConfig.ToSharedRef(), Writer);

							SDK->RoomV2->UpdateRoomConfig(
								Room.RoomId,
								RoomConfigString,
								UHathoraSDKRoomV2::FHathoraOnUpdateRoomConfig::CreateLambda(
									[](const FHathoraUpdateRoomConfigResult &Result)
									{
										if (Result.StatusCode != 200)
										{
											UE_LOG(LogHathoraSDK, Error, TEXT("Failed to update room config: %s"), *Result.ErrorMessage);
											FPlatformMisc::RequestExitWithStatus(true, 1);
										};
									}
								)
							);
						}
					}
				)
			);
		}
	}
#endif
}

FString UHathoraEngineSubsystem::WaitForRoomAllocation()
{
	UHathoraSDK *SDK = UHathoraSDK::CreateHathoraSDK();
	FHathoraServerEnvironment HathoraEnvVars = UHathoraSDK::GetServerEnvironment();

	FString RoomId;

	while (RoomId == TEXT(""))
	{
		FEvent* EventGetRooms = FPlatformProcess::GetSynchEventFromPool();

		SDK->RoomV2->GetActiveRoomsForProcess(
			HathoraEnvVars.ProcessId,
			UHathoraSDKRoomV2::FHathoraOnGetRoomsForProcess::CreateLambda(
				[EventGetRooms, SDK, &RoomId, this](const FHathoraGetRoomsForProcessResult &Result)
				{
					if (Result.StatusCode == 200)
					{
						for (int32 i = 0; i < Result.Data.Num(); i++)
						{
							FHathoraProcessRoomInfo Room = Result.Data[i];
							if (Room.Status == EHathoraRoomStatus::Active)
							{
								TSharedPtr<FJsonObject> OutRoomConfig;
								TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Room.RoomConfig);
								FJsonSerializer::Deserialize(Reader, OutRoomConfig);

								int32 RoomConfigPort;
								if (OutRoomConfig->TryGetNumberField(TEXT("hathoraGamePort"), RoomConfigPort))
								{
									if (RoomConfigPort != 0)
									{
										// this room is already allocated, skip it
										continue;
									}
								}

								// this room is not allocated yet (either has no port or has port 0)
								OutRoomConfig->SetNumberField(TEXT("hathoraGamePort"), Port);
								FString RoomConfigString;
								TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RoomConfigString);
								FJsonSerializer::Serialize(OutRoomConfig.ToSharedRef(), Writer);

								FEvent* EventUpdateConfig = FPlatformProcess::GetSynchEventFromPool();

								SDK->RoomV2->UpdateRoomConfig(
									Room.RoomId,
									RoomConfigString,
									UHathoraSDKRoomV2::FHathoraOnUpdateRoomConfig::CreateLambda(
										[EventUpdateConfig](const FHathoraUpdateRoomConfigResult &Result)
										{
											if (Result.StatusCode != 200)
											{
												UE_LOG(LogHathoraSDK, Error, TEXT("Failed to update room config: %s"), *Result.ErrorMessage);
												FPlatformMisc::RequestExitWithStatus(true, 1);
											}

											EventUpdateConfig->Trigger();
										}
									)
								);

								EventUpdateConfig->Wait();

								FPlatformProcess::ReturnSynchEventToPool(EventUpdateConfig);

								RoomId = Room.RoomId;
								break;
							}
						}
					}

					EventGetRooms->Trigger();
				}
			)
		);

		EventGetRooms->Wait();

		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			[EventGetRooms]()
			{
				EventGetRooms->Trigger();
			},
			5.0f,
			false
		);

		EventGetRooms->Wait();

		FPlatformProcess::ReturnSynchEventToPool(EventGetRooms);
	}

	return RoomId;
}

void UHathoraEngineSubsystem::Deinitialize()
{
	//
}

int32 UHathoraEngineSubsystem::GetPortFromRoomConfig(const FString &RoomConfig)
{
	TSharedPtr<FJsonObject> OutRoomConfig;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RoomConfig);
	FJsonSerializer::Deserialize(Reader, OutRoomConfig);

	int32 ParsedPort;
	if (OutRoomConfig->TryGetNumberField(TEXT("hathoraGamePort"), ParsedPort))
	{
		return ParsedPort;
	}

	return 0;

}

FString UHathoraEngineSubsystem::AddPortToRoomConfig(const FString &RoomConfig)
{
	TSharedPtr<FJsonObject> OutRoomConfig;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RoomConfig);
	FJsonSerializer::Deserialize(Reader, OutRoomConfig);

	OutRoomConfig->SetNumberField(TEXT("hathoraGamePort"), Port);
	FString RoomConfigString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RoomConfigString);
	FJsonSerializer::Serialize(OutRoomConfig.ToSharedRef(), Writer);

	return RoomConfigString;
}

FString UHathoraEngineSubsystem::RemovePortFromRoomConfig(const FString &RoomConfig)
{
	TSharedPtr<FJsonObject> OutRoomConfig;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RoomConfig);
	FJsonSerializer::Deserialize(Reader, OutRoomConfig);

	if (OutRoomConfig->HasField(TEXT("hathoraGamePort")))
	{
		OutRoomConfig->RemoveField(TEXT("hathoraGamePort"));
	}

	FString RoomConfigString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RoomConfigString);
	FJsonSerializer::Serialize(OutRoomConfig.ToSharedRef(), Writer);

	return RoomConfigString;
}
