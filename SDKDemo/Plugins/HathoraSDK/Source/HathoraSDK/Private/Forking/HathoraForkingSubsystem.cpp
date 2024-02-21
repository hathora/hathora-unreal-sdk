// Copyright 2024 Hathora, Inc.

#include "Forking/HathoraForkingSubsystem.h"
#include "Forking/HathoraForkProcess.h"
#include "HathoraSDKModule.h"
#include "HathoraSDK.h"
#include "HathoraSDKConfig.h"
#include "HathoraTypes.h"
#include "HathoraSDKRoomV2.h"
#include "HathoraSDKProcessesV1.h"
#include "HttpModule.h"
#include "HttpManager.h"

#if PLATFORM_LINUX
#include <fcntl.h>
#include <semaphore.h>
#endif

// This is the JSON key that the port will be stored under in the room config
const FString UHathoraForkingSubsystem::RoomConfigPortKey(TEXT("hathoraGamePort"));

// This is the default port name that will be used for the first child process. You can
// change this below if you want to use the same naming convention as the additional ports
// (e.g. "game0" and set the "default" port to something else)
const FString UHathoraForkingSubsystem::DefaultPortName(TEXT("default"));

// This is the prefix that will be used for the port name for the additional child processes.
// This will be followed by the index of the child process (1-based, e.g. "game1", "game2")
const FString UHathoraForkingSubsystem::ExposedPortNamePrefix(TEXT("game"));

// This is how much time we will wait between polling GetActiveRoomsForProcess to check if
// a room has been allocated to this process yet (in seconds).
const float UHathoraForkingSubsystem::RoomAllocationPollingInterval(1.0f);

void UHathoraForkingSubsystem::Initialize(FSubsystemCollectionBase &Collection)
{
	Super::Initialize(Collection);

	// This is the port provided to the master process, which could be overriden
	// via config or the -port command line parameter (default is 7777). We do this
	// here to make the original StartingPort available to children without having
	// a hardcoded value.
	FURL DefaultUrl;
	StartingPort = DefaultUrl.Port;
	ChildIdx = -1;

	UHathoraSDKConfig *Config = GetMutableDefault<UHathoraSDKConfig>();

	if (Config->GetUseBuiltInForking())
	{
		// You could move this call to a later point in your game's lifecycle if you
		// want to fork at a different time (i.e. after finished loading most of your game's
		// assets to share the memory across the processes).
		Fork();
	}
}

void UHathoraForkingSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UHathoraForkingSubsystem::Fork()
{
	// Forking is only supported on Linux (technically the UnixPlatformProcess handles this,
	// but Epic has stated "Linux" in the forking article (see the ../../Public/Forking/README.md))
#if PLATFORM_LINUX
	UWorld *World = GetGameInstance()->GetWorld();

	if (IsValid(World) && World->GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		FHathoraServerEnvironment HathoraEnvVars = UHathoraSDK::GetServerEnvironment();

		// This will fill an array of all the Hathora-exposed ports for the child processes.
		// We use these ports in the RoomConfig to let clients know which port to connect to.
		// This function blocks.
		GetExposedPorts();

		if (ExposedPorts.Num() != HathoraEnvVars.RoomsPerProcess)
		{
			UE_LOG(LogHathoraSDK, Error, TEXT("The number of exposed ports (%d) does not match the number of rooms per process (%d), will not attempt to fork"), ExposedPorts.Num(), HathoraEnvVars.RoomsPerProcess);
			return;
		}

		// We only want to fork if the RoomsPerProcess environment variable
		// (injected by Hathora) is greater than 1
		if (HathoraEnvVars.RoomsPerProcess > 1) {
			// We can tell the underlying engine forking utility to set the command line parameters
			// for each child by placing text files in the `-WaitAndForkCmdLinePath` directory. We
			// are putting these in the below directory for them to be read from.
			FString BasePath = FPaths::Combine(
				FPaths::ProjectSavedDir(),
				TEXT("Hathora"),
				TEXT("RoomConsoleVars")
			);

			// We cannot inject these parameters like we do below because the helper functions
			// below will set globally static variables before we can inject them, making the
			// changes ineffective. You must provide these parameters to the server executable
			// yourself. You can see the Dockerfile in the SDKDemo repo for an example line.
			if (FPlatformProcess::SupportsMultithreading() || !FForkProcessHelper::SupportsMultithreadingPostFork())
			{
				UE_LOG(LogHathoraSDK, Error, TEXT("You must provide `-nothreading` and `-PostForkThreading` parameters to the server executable to use multiple rooms per process; will not attempt to fork."));
				return;
			}

			// We need to inject these parameters onto the command line (which is globally static)
			// because FUnixPlatformProcess::WaitAndFork() is expecting them to enable forking.
			FCommandLine::Append(*FString(TEXT(" -NumForks=") + FString::FromInt(HathoraEnvVars.RoomsPerProcess)));
			FCommandLine::Append(TEXT(" -WaitAndFork"));
			FCommandLine::Append(*FString(TEXT(" -WaitAndForkCmdLinePath=") + BasePath));

			UE_LOG(LogHathoraSDK, Log, TEXT("Forking %d processes"), HathoraEnvVars.RoomsPerProcess);

			// Here we set the command line parameters for each child process. Each file must have
			// a 1-based indexed name (e.g. 1, 2, 3, etc.). We specify the incremented port
			// and the `-PostForkThreading` to enable multithreading in the child processes.
			for (int32 i = 0; i < HathoraEnvVars.RoomsPerProcess; i++)
			{
				FString RoomConsoleVarsPath = FPaths::Combine(
					BasePath,
					FString::Printf(TEXT("%d"), i + 1)
				);

				FFileHelper::SaveStringToFile(TEXT("-port=") + FString::FromInt(StartingPort + i) + TEXT(" -PostForkThreading"), *RoomConsoleVarsPath);
			}

			// This function is designed to be blocking until we have a room ID and
			// the game can continue to run. To ensure this happens, we need to make
			// sure only one child process tries to assign an allocated room at once.
			// We achieve this with a Linux semaphore. This function could go before
			// or after the fork, but we decided to create the semaphore in the parent
			// for simplicity.
			errno = 0;
			sem_t* sem = sem_open("/hathora-room-allocation", O_CREAT, 0644, 1);
			if (sem == SEM_FAILED)
			{
				UE_LOG(LogHathoraSDK, Error, TEXT("Failed to create semaphore: %d"), errno);
				return;
			}

			// F[Hathora]ForkProcess::ForkIfRequested is a wrapper around the
			// UnixPlatformProcess::WaitAndFork, which is wrapper around the actual fork()
			// call. For the purposes of this class, this call will fork the parent process
			// into all of the child processes. The parent process will not return from this
			// until the engine is being shut down for all processes. The children processes
			// will continue on. The parent process will also monitor for terminated children
			// and refork them again if necessary.
			FHathoraForkProcess::ForkIfRequested(World);

			if (!FForkProcessHelper::IsForkedChildProcess())
			{
				// The parent/master process doesn't return from FHathoraForkProcess::ForkIfRequested
				// until the engine is being shut down for all processes, but it will eventually return.
				// We don't want to do any of the below on the parent process so let's just return.
				return;
			}

			// Only children processes will get to this point

			// FURL's default construction will change automatically for forked processes because
			// of the injected -port command line parameter for each child. By simply constructing
			// FURL here, we can get the port number for the child process. We can use this to
			// calculate an index for the child process since they're 0-based incremented from the
			// starting port (which we set in the parent process).
			FURL DefaultUrl2;
			ChildIdx = DefaultUrl2.Port - StartingPort;
			UE_LOG(LogHathoraSDK, Log, TEXT("[Child %d] Forked process started with port: %d"), ChildIdx, DefaultUrl2.Port);

			// Here is where we wait for the semaphore to be available. This will block
			// individual child processes until it's their turn to try to assign an allocated
			// room.
			UE_LOG(LogHathoraSDK, Log, TEXT("[Child %d] Waiting to acquire the room allocation semaphore"), ChildIdx);
			if (sem_wait(sem) == -1)
			{
				UE_LOG(LogHathoraSDK, Error, TEXT("[Child %d] There was an error when trying to acquire the semaphore"), ChildIdx);
				return;
			}

			UE_LOG(LogHathoraSDK, Log, TEXT("[Child %d] Acquired the room allocation semaphore, waiting for room allocation"), ChildIdx);

			// Normally the F[Hathora]ForkProcess class provided by Epic would call this function
			// once after the frame finished. This function does some setup including multithreading.
			// Since this Fork() function is blocking, we don't really finish the frame before needing
			// the HTTP thread to be initiated. We need to call this function ourselves to ensure
			// the setup happens, and it won't be called again.
			FHathoraForkProcess::OnEndFramePostFork();

			// Block for this child until there is a room that hasn't been assigned to this process.
			// We denote how this is determined within WaitForRoomAllocation by checking the RoomConfig
			// for UHathoraForkingSubsystem::RoomConfigPortKey.
			WaitForRoomAllocation();

			UE_LOG(LogHathoraSDK, Log, TEXT("[Child %d] Room allocation complete, releasing semaphore"), ChildIdx);
			if (sem_post(sem) == -1)
			{
				UE_LOG(LogHathoraSDK, Error, TEXT("[Child %d] There was an error when trying to release the semaphore"), ChildIdx);
				return;
			}
		}
		else
		{
			// If we only have one room per process, we don't need to fork, but we can still
			// use the same room allocation mechanism as the forked processes.
			ChildIdx = 0;
			WaitForRoomAllocation();
		}
	}
	else
	{
		if (!IsValid(World))
		{
			UE_LOG(LogHathoraSDK, Log, TEXT("HathoraForkingSubsystem: World is not valid yet"));
		}
		else
		{
			UE_LOG(LogHathoraSDK, Log, TEXT("HathoraForkingSubsystem: Not a dedicated server"));
		}
	}
#endif
}

// This function calls GetProcessInfo once on the parent process to get the exposed ports for
// the child processes. This will be put into a 0-based array that the child processes will use
// to let clients know which port to connect to. This function blocks.
void UHathoraForkingSubsystem::GetExposedPorts()
{
	ExposedPorts = TArray<int32>();
	SDK = UHathoraSDK::CreateHathoraSDK();
	FEvent* Event = FPlatformProcess::GetSynchEventFromPool();
	FHathoraServerEnvironment HathoraEnvVars = UHathoraSDK::GetServerEnvironment();

	SDK->ProcessesV1->GetProcessInfo(
		HathoraEnvVars.ProcessId,
		UHathoraSDKProcessesV1::FHathoraOnProcessInfo::CreateLambda(
			[Event, HathoraEnvVars, this](const FHathoraProcessInfoResult &Result)
			{
				if (Result.StatusCode == 200)
				{
					UE_LOG(LogHathoraSDK, Log, TEXT("Got process info, id: %s"), *Result.Data.ProcessId);

					// Get the exposed port for the first child process. If its name is "default"
					// then it will be on ExposedPort, otherwise, fetch it from AdditionalExposedPorts
					if (UHathoraForkingSubsystem::DefaultPortName == TEXT("default"))
					{
						ExposedPorts.Add(Result.Data.ExposedPort.Port);
					}
					else
					{
						for (int32 i = 0; i < Result.Data.AdditionalExposedPorts.Num(); i++)
						{
							FHathoraExposedPort ExposedPort = Result.Data.AdditionalExposedPorts[i];
							if (ExposedPort.Name == UHathoraForkingSubsystem::DefaultPortName)
							{
								ExposedPorts.Add(ExposedPort.Port);
							}
						}
					}

					// While we could omit the double loop since the port order doesn't technically matter,
					// having two loops here is likely worth the negligible performance cost to ensure that
					// ChildIdx matches the index used in the port name for debugging purposes. Start at i=1
					// since the first port is already added.
					for (int32 i = 1; i < HathoraEnvVars.RoomsPerProcess; i++)
					{
						for (int32 j = 0; j < Result.Data.AdditionalExposedPorts.Num(); j++)
						{
							FHathoraExposedPort ExposedPort = Result.Data.AdditionalExposedPorts[j];
							if (
								ExposedPort.Name.StartsWith(UHathoraForkingSubsystem::ExposedPortNamePrefix) &&
								ExposedPort.Name.EndsWith(FString::FromInt(i))
							)
							{
								ExposedPorts.Add(ExposedPort.Port);
							}
						}
					}
				}
				else
				{
					UE_LOG(LogHathoraSDK, Error, TEXT("Failed to get process info: %s"), *Result.ErrorMessage);
				}

				Event->Trigger();
			}
		)
	);

	// This will wait for the event to be triggered, which will happen when the HTTP request
	// returns. We will wait for 10ms at a time to allow the HTTP thread to tick.
	while(!Event->Wait(10))
	{
		// We need to tick the HTTP thread to allow the HTTP request to return.
		FHttpModule::Get().GetHttpManager().Tick(0.1);
	}

	// Return the event to the pool
	FPlatformProcess::ReturnSynchEventToPool(Event);
}

void UHathoraForkingSubsystem::WaitForRoomAllocation()
{
	SDK = UHathoraSDK::CreateHathoraSDK();
	FHathoraServerEnvironment HathoraEnvVars = UHathoraSDK::GetServerEnvironment();

	// Block until we have a RoomId
	while (RoomId == TEXT(""))
	{
		// This Event allows us to block for the asynchronous HTTP methods to return.
		// We call Event->Wait(millisecondsToWait) to block until the event is triggered
		// with Event->Trigger().
		FEvent* Event = FPlatformProcess::GetSynchEventFromPool();

		// We use this variable to later update the room config with the port
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

								// If the RoomConfig isn't a valid JSON string, then we will just create
								// a new one to update the room config with the port.
								if (!OutRoomConfig.IsValid())
								{
									OutRoomConfig = MakeShareable(new FJsonObject());
								}

								// Here is where we determine whether or not a room is already assigned to a
								// forked child process. If it has a non-zero port, then we will skip
								// this room and continue to the next one.
								int32 RoomConfigPort = 0;
								if (OutRoomConfig->TryGetNumberField(
									UHathoraForkingSubsystem::RoomConfigPortKey,
									RoomConfigPort
								))
								{
									if (RoomConfigPort != 0)
									{
										// this room is already allocated, skip it
										UE_LOG(LogHathoraSDK, Log, TEXT("Room already allocated, id: %s, config: %s"), *Room.RoomId, *Room.RoomConfig);
										continue;
									}
								}

								// This room is not allocated yet, so we will update the room config with
								// the port for this child process to "assign" it.
								OutRoomConfig->SetNumberField(
									UHathoraForkingSubsystem::RoomConfigPortKey,
									ExposedPorts[ChildIdx]
								);

								TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&NewRoomConfigString);
								FJsonSerializer::Serialize(OutRoomConfig.ToSharedRef(), Writer);

								RoomId = Room.RoomId;

								break;
							}
						}
					}

					// Regardless if we found a room or not, we need to trigger the event to unblock.
					Event->Trigger();
				}
			)
		);

		// This will wait for the event to be triggered, which will happen when the HTTP request
		// returns. We will wait for 10ms at a time to allow the HTTP thread to tick.
		while(!Event->Wait(10))
		{
			// We need to tick the HTTP thread to allow the HTTP request to return.
			FHttpModule::Get().GetHttpManager().Tick(0.1);
		}

		// If we found a valid room and created a new room config string, we will update the room
		// config with the port.
		if (RoomId != TEXT("") && NewRoomConfigString != TEXT(""))
		{
			UE_LOG(LogHathoraSDK, Log, TEXT("Found room, id: %s, config: %s"), *RoomId, *NewRoomConfigString);

			// Allow the waitable again
			Event->Reset();

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

			// Wait for the UpdateRoomConfig call to return
			while(!Event->Wait(10))
			{
				FHttpModule::Get().GetHttpManager().Tick(0.1);
			}
		}

		// Return the event to the pool
		FPlatformProcess::ReturnSynchEventToPool(Event);

		// Sleep to give some time between checking the rooms again
		if (RoomId == TEXT(""))
		{
			FPlatformProcess::Sleep(UHathoraForkingSubsystem::RoomAllocationPollingInterval);
		}
	}

	UE_LOG(LogHathoraSDK, Log, TEXT("Room allocation complete, id: %s"), *RoomId);
}

// This function is used to extract the port from the RoomConfig JSON string, which
// is used in UHathoraLobbyComponent to let clients know which port to connect to.
// Returns 0 if the port is not found.
int32 UHathoraForkingSubsystem::GetPortFromRoomConfig(const FString &RoomConfig)
{
	TSharedPtr<FJsonObject> OutRoomConfig;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RoomConfig);
	FJsonSerializer::Deserialize(Reader, OutRoomConfig);

	if (!OutRoomConfig.IsValid())
	{
		return 0;
	}

	int32 ParsedPort = 0;
	if (OutRoomConfig->TryGetNumberField(
		UHathoraForkingSubsystem::RoomConfigPortKey,
		ParsedPort
	))
	{
		return ParsedPort;
	}

	return 0;
}

// This function is used to remove the port from the RoomConfig JSON string. This is
// useful if you want your customized RoomConfig without the injected port.
FString UHathoraForkingSubsystem::RemovePortFromRoomConfig(const FString &RoomConfig)
{
	TSharedPtr<FJsonObject> OutRoomConfig;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RoomConfig);
	FJsonSerializer::Deserialize(Reader, OutRoomConfig);

	if (!OutRoomConfig.IsValid())
	{
		return RoomConfig;
	}

	if (OutRoomConfig->HasField(UHathoraForkingSubsystem::RoomConfigPortKey))
	{
		OutRoomConfig->RemoveField(UHathoraForkingSubsystem::RoomConfigPortKey);
	}

	FString RoomConfigString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RoomConfigString);
	FJsonSerializer::Serialize(OutRoomConfig.ToSharedRef(), Writer);

	return RoomConfigString;
}

// This function is used to add the port to the RoomConfig JSON string. This is primarily
// used in the UDemoRoomConfigFunctionLibrary sample class (in this Hathora SDK sample repo,
// not included in the plugin itself) which will inject the port when serializing the customized
// room config (as to not overwrite the injected port). Using this function only makes
// sense on a dedicated server using the built-in forking.
FString UHathoraForkingSubsystem::AddPortToRoomConfig(const FString &RoomConfig)
{
#if WITH_SERVER_CODE
	TSharedPtr<FJsonObject> OutRoomConfig;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RoomConfig);
	FJsonSerializer::Deserialize(Reader, OutRoomConfig);

	if (!OutRoomConfig.IsValid())
	{
		return RoomConfig;
	}

	OutRoomConfig->SetNumberField(
		UHathoraForkingSubsystem::RoomConfigPortKey,
		ExposedPorts[ChildIdx]
	);

	FString RoomConfigString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RoomConfigString);
	FJsonSerializer::Serialize(OutRoomConfig.ToSharedRef(), Writer);

	return RoomConfigString;
#else
	return RoomConfig;
#endif
}
