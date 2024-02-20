// Copyright 2024 Hathora, Inc.

#include "HathoraGameInstanceSubsystem.h"
#include "HathoraSDKModule.h"
#include "HathoraSDK.h"
#include "HathoraTypes.h"
#include "HathoraForkProcess.h"
#include "HathoraSDKRoomV2.h"
#include "HttpModule.h"
#include "HttpManager.h"

#if PLATFORM_LINUX
#include <fcntl.h>
#include <semaphore.h>
#endif

void UHathoraGameInstanceSubsystem::Initialize(FSubsystemCollectionBase &Collection)
{
	Super::Initialize(Collection);

	UWorld *World = GetGameInstance()->GetWorld();

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

			if (FPlatformProcess::SupportsMultithreading() || !FForkProcessHelper::SupportsMultithreadingPostFork())
			{
				UE_LOG(LogHathoraSDK, Error, TEXT("You must provide `-nothreading` and `-PostForkThreading` parameters to the server executable to use multiple rooms per process; will not attempt to fork."));
				return;
			}

			FCommandLine::Append(*FString(TEXT(" -NumForks=") + FString::FromInt(HathoraEnvVars.RoomsPerProcess)));
			FCommandLine::Append(TEXT(" -WaitAndFork"));
			FCommandLine::Append(*FString(TEXT(" -WaitAndForkCmdLinePath=") + BasePath));

			UE_LOG(LogHathoraSDK, Log, TEXT("Forking %d processes"), HathoraEnvVars.RoomsPerProcess);

			for (int32 i = 0; i < HathoraEnvVars.RoomsPerProcess; i++)
			{
				FString RoomConsoleVarsPath = FPaths::Combine(
					BasePath,
					FString::Printf(TEXT("%d"), i + 1)
				);

				FFileHelper::SaveStringToFile(TEXT("-port=") + FString::FromInt(7777 + i) + TEXT(" -PostForkThreading"), *RoomConsoleVarsPath);
			}

			// Create a named semaphore
			errno = 0;
			sem_t* sem = sem_open("/hathora-room-allocation", O_CREAT, 0644, 1);
			if (sem == SEM_FAILED)
			{
				UE_LOG(LogHathoraSDK, Error, TEXT("Failed to create semaphore: %d"), errno);
				return;
			}

			GLog->Flush();
			FHathoraForkProcess::ForkIfRequested(World);
			GLog->Flush();

			// only children should get to this point
			FURL DefaultUrl2;
			ChildIdx = DefaultUrl2.Port - 7777;
			UE_LOG(LogHathoraSDK, Log, TEXT("[Child %d] Forked process started with port: %d"), ChildIdx, DefaultUrl2.Port);

			UE_LOG(LogHathoraSDK, Log, TEXT("[Child %d] Waiting to acquire the room allocation semaphore"), ChildIdx);
			GLog->Flush();
			if (sem_wait(sem) == -1)
			{
				UE_LOG(LogHathoraSDK, Error, TEXT("[Child %d] There was an error when trying to acquire the semaphore"), ChildIdx);
				return;
			}

			UE_LOG(LogHathoraSDK, Log, TEXT("[Child %d] Acquired the room allocation semaphore, waiting for room allocation"), ChildIdx);
			GLog->Flush();

			FHathoraForkProcess::OnEndFramePostFork();
			WaitForRoomAllocation();

			UE_LOG(LogHathoraSDK, Log, TEXT("[Child %d] Room allocation complete, releasing semaphore"), ChildIdx);
			GLog->Flush();

			if (sem_post(sem) == -1)
			{
				UE_LOG(LogHathoraSDK, Error, TEXT("[Child %d] There was an error when trying to release the semaphore"), ChildIdx);
				return;
			}
		}
		else
		{
			WaitForRoomAllocation();
		}
	}
	else
	{
		if (!IsValid(World))
		{
			UE_LOG(LogHathoraSDK, Log, TEXT("HathoraGameInstanceSubsystem: World is not valid yet"));
		}
		else
		{
			UE_LOG(LogHathoraSDK, Log, TEXT("HathoraGameInstanceSubsystem: Not a dedicated server"));
		}
	}
#endif
}

void UHathoraGameInstanceSubsystem::WaitForRoomAllocation()
{
	SDK = UHathoraSDK::CreateHathoraSDK();
	FHathoraServerEnvironment HathoraEnvVars = UHathoraSDK::GetServerEnvironment();

	while (RoomId == TEXT(""))
	{
		FEvent* Event = FPlatformProcess::GetSynchEventFromPool();

		FString NewRoomConfigString;

		UE_LOG(LogHathoraSDK, Log, TEXT("Looking for an allocated room"));

		SDK->RoomV2->GetActiveRoomsForProcess(
			HathoraEnvVars.ProcessId,
			UHathoraSDKRoomV2::FHathoraOnGetRoomsForProcess::CreateLambda(
				[Event, &NewRoomConfigString, this](const FHathoraGetRoomsForProcessResult &Result)
				{
					UE_LOG(LogHathoraSDK, Log, TEXT("Got a response for rooms for process, status code: %d"), Result.StatusCode);

					if (Result.StatusCode == 200)
					{
						UE_LOG(LogHathoraSDK, Log, TEXT("Num rooms: %d"), Result.Data.Num());
						for (int32 i = 0; i < Result.Data.Num(); i++)
						{
							FHathoraProcessRoomInfo Room = Result.Data[i];
							if (Room.Status == EHathoraRoomStatus::Active)
							{
								TSharedPtr<FJsonObject> OutRoomConfig;
								TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Room.RoomConfig);
								FJsonSerializer::Deserialize(Reader, OutRoomConfig);

								if (!OutRoomConfig.IsValid())
								{
									OutRoomConfig = MakeShareable(new FJsonObject());
								}

								FString RoomConfigPortName;
								if (OutRoomConfig->TryGetStringField(TEXT("hathoraGamePortName"), RoomConfigPortName))
								{
									if (RoomConfigPortName != TEXT(""))
									{
										// this room is already allocated, skip it
										UE_LOG(LogHathoraSDK, Log, TEXT("Room already allocated, id: %s, config: %s"), *Room.RoomId, *Room.RoomConfig);
										continue;
									}
								}

								// this room is not allocated yet (either has no port or has port 0)
								FURL DefaultUrl;
								if (DefaultUrl.Port == 7777)
								{
									OutRoomConfig->SetStringField(TEXT("hathoraGamePortName"), TEXT("default"));
								}
								else
								{
									OutRoomConfig->SetStringField(TEXT("hathoraGamePortName"), TEXT("fork") + FString::FromInt(DefaultUrl.Port - 7777));
								}
								TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&NewRoomConfigString);
								FJsonSerializer::Serialize(OutRoomConfig.ToSharedRef(), Writer);

								RoomId = Room.RoomId;

								break;
							}
						}
					}

					Event->Trigger();
				}
			)
		);

		while(!Event->Wait(10))
		{
			FHttpModule::Get().GetHttpManager().Tick(0.1);
		}

		if (RoomId != TEXT("") && NewRoomConfigString != TEXT(""))
		{
			UE_LOG(LogHathoraSDK, Log, TEXT("Found room, id: %s, config: %s"), *RoomId, *NewRoomConfigString);
			FPlatformProcess::ReturnSynchEventToPool(Event);
			Event = FPlatformProcess::GetSynchEventFromPool();

			SDK->RoomV2->UpdateRoomConfig(
				RoomId,
				NewRoomConfigString,
				UHathoraSDKRoomV2::FHathoraOnUpdateRoomConfig::CreateLambda(
					[Event](const FHathoraUpdateRoomConfigResult &Result)
					{
						if (Result.StatusCode != 204)
						{
							UE_LOG(LogHathoraSDK, Error, TEXT("Failed to update room config: %s"), *Result.ErrorMessage);
						}

						Event->Trigger();
					}
				)
			);

			while(!Event->Wait(10))
			{
				FHttpModule::Get().GetHttpManager().Tick(0.1);
			}
		}

		FPlatformProcess::ReturnSynchEventToPool(Event);

		FPlatformProcess::Sleep(1.0f);
	}

	UE_LOG(LogHathoraSDK, Log, TEXT("Room allocation complete, id: %s"), *RoomId);
}

void UHathoraGameInstanceSubsystem::Deinitialize()
{
	Super::Deinitialize();
}
