// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Delegates/IDelegateInstance.h"

class UWorld;

/** Class to manage forking dedicated servers in Linux */
class FHathoraForkProcess
{
public:
	/**
	* This is a function that halts execution and waits for signals to cause forked processes to be created and continue execution.
	* This forking behavior is only allowed on linux servers, but note that this function needs to get called even if you are not forking so perf counters properly get created.
	* Forking also is only allowed when running with -nothreading, and only happens is -WaitAndFork is specified on the command line.
	* It will fork child processes that use the shared memory from this process.
	* This will only work for the parent process, child processes cannot fork further.
	* The parent process will return when GIsRequestingExit is true. SIGRTMIN+1 is used to cause a fork to happen.
	* If sigqueue is used, the payload int will be split into the upper and lower uint16 values. The upper value is a "cookie" and the
	*     lower value is an "index". These two values will be used to name the process using the pattern DS-<cookie>-<index>. This name
	*     can be used to uniquely discover the process that was spawned.
	* If -NumForks=x is suppled on the command line, x forks will be made when the function is called.
	* If -WaitAndForkCmdLinePath=Foo is suppled, the command line parameters of the child processes will be filled out with the contents
	*     of files found in the directory referred to by Foo, where the child's "index" is the name of the file to be read in the directory.
	* If -WaitAndForkRequireResponse is on the command line, child processes will not proceed after being spawned until a SIGRTMIN+2 signal is sent to them.
	* A world is required to shut down the current net driver and to get the game instance to set up perf counters.
	*/
	static void ForkIfRequested(UWorld* World);

	/** Should the process exit when the last player has left or should the process reload and create a new session */
	static bool ShutdownProcessAtSessionEnd();


	/** Set up this process to ask the parent process to shut down when this one exits */
	static void MarkShouldCloseParentProcessWhenShuttingDown();

public:

	/** Handle used to remove the OnEndFrame delegate after it is triggered once */
	static FDelegateHandle OnEndFrameHandle;

	/** Delegate called immediately after forking */
	static void OnEndFramePostFork();

private:
	/** Only attempt to fork exactly once. Processes should not attempt to fork again after the first request, child or parent. */
	static bool bForkAlreadyAttempted;

	/** If true, when this process exists, it will return an exit code to indicate that the parent process should also shut down */
	static bool bShutdownWithExitCodeToCloseParent;
};