# Hathora Forking / Multiple Rooms Per Process

This is an overview of what these additional classes provide and how to use them.

## Overview

To enable more than 1 room per "Hathora Process" (aka Number of rooms per process > 1) within Unreal, we need to accomplish these tasks:

1. Fork the Unreal process into multiple processes
1. For each child process, listen on a different local port
1. Provide a mechanism for clients to know which exposed/public port (which is assigned by Hathora) to connect to for a given Room ID

Epic Games has documented about supporting this in [this article](https://forums.unrealengine.com/t/tech-note-the-fforkprocess-class-for-managing-forking-dedicated-servers-in-linux/264963). This article provides the `FForkProcess.h` that is mentioned in a more broad [article about hosting advice they did](https://forums.unrealengine.com/docs?search=Server%20Hosting%20Advice&topic=265031).

This class provides a helper function to wrap around the UE built-in mechanisms for forking.

Note that forking is only supported for **Linux Dedicated Servers** (as `FUnixPlatformProcess` is the only platform that implements the `WaitAndFork()` method).

## Forking in Unreal

From a birds-eye view, here is roughly how forking is implemented in these classes.

1. [`UHathoraForkingSubsystem::Fork`](./HathoraForkingSubsystem.h) calls [`FHathoraForkProcess::ForkIfRequested`](./HathoraForkProcess.h).
	- Note: `FHathoraForkProcess` is the same as the provided `FForkProcess` from the forementioned article. We renamed it to not conflict if you copy it separately. This class helps with forking, but Epic never published it as part of Unreal.
1. [`FHathoraForkProcess::ForkIfRequested`](./HathoraForkProcess.h) calls `FPlatformProcess::WaitAndFork`, which if you're using Linux calls [`FUnixPlatformProcess::WaitAndFork`](https://github.com/EpicGames/UnrealEngine/blob/release/Engine/Source/Runtime/Core/Public/Unix/UnixPlatformProcess.h)
1. `FUnixPlatformProcess::WaitAndFork` calls the system call `fork()` for the `-NumForks` command line parameter (which we automatically inject into the command line in `UHathoraForkingSubsystem` to equal the environment variable `HATHORA_ROOMS_PER_PROCESS` injected by Hathora).
1. After the parent process forks `-NumForks` times, it will then continue to monitor and wait for children processes to terminate. If they do terminate with anything other than a special exit code, it will fork for them again.
1. Each child process will reinitialize their network stack again, taking into account any of the command line parameters  specified in the folder denoted by the `-WaitAndForkCmdLinePath` command line parameter (also automatically injected by `UHathoraForkingSubsystem`). There needs to be a file for each child process in this folder with the name equal to the index of the child (1-based, e.g. "1", "2", "3", etc.). This is a text file with the command line parameters (e.g. `-port=7778`).
	- This is completely handled for you automatically in `UHathoraForkingSubsystem` by using an incrementing port for each child (e.g. 7777, 7778, 7779). If you specify `-port` on the master process command line or config file, that will be the base which the system will increment on.
1. The child process ends up continuing on in `UHathoraForkingSubsystem` where there is some subsequent Hathora Cloud API calls before continuing the server ticking

To use Unreal's forking system, you must specify `-nothreading` on the command line of the parent/master process. **We cannot automatically inject this command line parameter**, so you must manually modify your call (e.g. your entrypoint in your `Dockerfile`) to add this. This is just a hard requirement by the engine code; without it it will just not fork.

To use the provided `UHathoraForkingSubsystem`, you also need to specify `-PostForkThreading` command line parameter for the parent/master process to ensure the HTTP calls can happen on a separate thread.

We've provided an example of this in the [`Dockerfile`](../../../../../../Dockerfile) at the root of the Hathora SDK demo repo.

## UHathoraForkingSubsystem

The provided [`UHathoraForkingSubsystem`](./HathoraForkingSubsystem.h) class provides a full example of how to implement the full life cycle. You may need to modify this depending on any third-party vendors.

This is a singleton class that will initialize during engine startup. See the [Unreal documentation on Subsystems](https://docs.unrealengine.com/5.3/en-US/programming-subsystems-in-unreal-engine/) for more details there. `UHathoraForkingSubsystem` is a `UGameInstanceSubsystem` to ensure it loads when the `UGameInstance` loads.

`UHathoraForkingSubsystem` provides a public `void Fork()` function that will start the forking process. This is a blocking function and should be called when you want to separate the parent and child processes. By default, `UHathoraForkingSubsystem` calls this in its `Initialize()` function if the [project setting](#enabling-via-project-settings) is enabled.

This `Fork()` method will do the following:

1. Fetch the Exposed Ports for the current Hathora Process (via the `GetProcessInfo` Hathora Cloud API call), saving them for children processes to use
1. Save each set of command line parameters for the children processes `-port=XXXX` and `-PostForkThreading` (note, that you still have to specify this in your startup command for the parent process; we inject it here too for the children process as the flag is used both by the parent and children processes).
1. Inject command line parameters for the parent process that enable the Unreal forking capability
1. Call `FHathoraForkProcess::ForkIfRequested` as mentioned [above](#forking-in-unreal)
1. Children then, one a time when they can acquire a semaphore, poll `GetActiveRoomsForProcess` API call until there is a room they can self-assign
1. Once there is a room, it will update its Room Config (assuming it is a JSON string) with a `hathoraGamePort` variable which is the public/exposed port for that child process/room
	- `UHathoraLobbyComponent` has logic in it to extract this variable if the [project setting](#enabling-via-project-settings) is enabled for the client and use that port instead of the default `exposedPort.port` variable from the `GetConnectionInfo` API call.
1. Then the child process unblocks and lets Unreal continue to tick

### Enabling via Project Settings

This functionality is disabled by default and must be enabled in your Project Settings. Under `Plugins > Hathora SDK`, enable `Use Built In Forking` to enable processing supporting forking functionality. You may need to click the `Set as Default` button to save the setting to the `Config/DefaultGame.ini` config file for it to be properly packaged in.

### More Technical Details

The [HathoraForkingSubsystem.cpp](../../Private/Forking/HathoraForkingSubsystem.cpp) source file has more comments on described behavior of how this all functions.