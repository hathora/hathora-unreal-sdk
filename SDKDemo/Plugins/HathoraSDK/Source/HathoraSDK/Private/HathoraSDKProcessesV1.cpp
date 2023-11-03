// Copyright 2023 Hathora, Inc.

#include "HathoraSDKProcessesV1.h"
#include "HathoraSDKModule.h"
#include "JsonUtilities.h"

void UHathoraSDKProcessesV1::GetAllRunningProcesses(FHathoraOnProcessInfos OnComplete)
{
	TArray<TPair<FString, FString>> QueryOptions;
	GetProcesses(true, QueryOptions, OnComplete);
}

void UHathoraSDKProcessesV1::GetRegionRunningProcesses(EHathoraCloudRegion Region, FHathoraOnProcessInfos OnComplete)
{
	TArray<TPair<FString, FString>> QueryOptions;
	QueryOptions.Add(TPair<FString, FString>(TEXT("region"), GetRegionString(Region)));
	GetProcesses(true, QueryOptions, OnComplete);
}

void UHathoraSDKProcessesV1::GetAllStoppedProcesses(FHathoraOnProcessInfos OnComplete)
{
	TArray<TPair<FString, FString>> QueryOptions;
	GetProcesses(false, QueryOptions, OnComplete);
}

void UHathoraSDKProcessesV1::GetRegionStoppedProcesses(EHathoraCloudRegion Region, FHathoraOnProcessInfos OnComplete)
{
	TArray<TPair<FString, FString>> QueryOptions;
	QueryOptions.Add(TPair<FString, FString>(TEXT("region"), GetRegionString(Region)));
	GetProcesses(false, QueryOptions, OnComplete);
}

void UHathoraSDKProcessesV1::GetProcesses(bool bRunning, TArray<TPair<FString, FString>> QueryOptions, FHathoraOnProcessInfos OnComplete)
{
	SendRequest(
		TEXT("GET"),
		FString::Printf(TEXT("/processes/v1/%s/list/%s"), *AppId, bRunning ? TEXT("running") : TEXT("stopped")),
		QueryOptions,
		[&, OnComplete](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) mutable
		{
			FHathoraProcessInfosResult Result;
			if (bSuccess && Response.IsValid())
			{
				Result.StatusCode = Response->GetResponseCode();
				FString Content = Response->GetContentAsString();

				if (Result.StatusCode == 200)
				{
					TArray<TSharedPtr<FJsonValue>> OutArray;
					TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
					FJsonSerializer::Deserialize(Reader, OutArray);

					for (TSharedPtr<FJsonValue> Value : OutArray)
					{
						TSharedPtr<FJsonObject> Object = Value->AsObject();
						Result.Data.Add(ParseProcessInfo(Object));
					}
				}
				else
				{
					Result.ErrorMessage = Content;
				}
			}
			else
			{
				Result.ErrorMessage = TEXT("Could not list processes, unknown error");
			}

			if (!Result.ErrorMessage.IsEmpty())
			{
				UE_LOG(LogHathoraSDK, Error, TEXT("[Get%sProcesses] Error: %s"), bRunning ? TEXT("Running") : TEXT("Stopped"), *Result.ErrorMessage);
			}

			if (!OnComplete.ExecuteIfBound(Result))
			{
				UE_LOG(LogHathoraSDK, Warning, TEXT("[Get%sProcesses] function pointer was not valid, so OnComplete will not be called"), bRunning ? TEXT("Running") : TEXT("Stopped"));
			}
		});
}

void UHathoraSDKProcessesV1::GetProcessInfo(FString ProcessId, FHathoraOnProcessInfo OnComplete)
{
	SendRequest(
		TEXT("GET"),
		FString::Printf(TEXT("/processes/v1/%s/info/%s"), *AppId, *ProcessId),
		[&, OnComplete](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) mutable
		{
			FHathoraProcessInfoResult Result;
			if (bSuccess && Response.IsValid())
			{
				Result.StatusCode = Response->GetResponseCode();
				FString Content = Response->GetContentAsString();

				if (Result.StatusCode == 200)
				{
					TSharedPtr<FJsonObject> OutObject;
					TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
					FJsonSerializer::Deserialize(Reader, OutObject);

					if (OutObject.IsValid())
					{
						Result.Data = ParseProcessInfo(OutObject);
					}
					else
					{
						Result.ErrorMessage = TEXT("Could not parse response");
					}
				}
				else
				{
					Result.ErrorMessage = Content;
				}
			}
			else
			{
				Result.ErrorMessage = TEXT("Could not list processes, unknown error");
			}

			if (!Result.ErrorMessage.IsEmpty())
			{
				UE_LOG(LogHathoraSDK, Error, TEXT("[GetProcessInfo] Error: %s"), *Result.ErrorMessage);
			}

			if (!OnComplete.ExecuteIfBound(Result))
			{
				UE_LOG(LogHathoraSDK, Warning, TEXT("[GetProcessInfo] function pointer was not valid, so OnComplete will not be called"));
			}
		});
}

FHathoraProcessInfo UHathoraSDKProcessesV1::ParseProcessInfo(TSharedPtr<FJsonObject> ProcessInfoJson)
{
	FHathoraProcessInfo ProcessInfo;

	ProcessInfo.EgressedBytes = ProcessInfoJson->GetIntegerField(TEXT("egressedBytes"));
	ProcessInfo.ActiveConnections = ProcessInfoJson->GetIntegerField(TEXT("activeConnections"));

	FDateTime::ParseIso8601(
		*ProcessInfoJson->GetStringField(TEXT("roomsAllocatedUpdatedAt")),
		ProcessInfo.RoomsAllocatedUpdatedAt
	);

	ProcessInfo.RoomsAllocated = ProcessInfoJson->GetIntegerField(TEXT("roomsAllocated"));
	ProcessInfo.bDraining = ProcessInfoJson->GetBoolField(TEXT("draining"));

	ProcessInfo.bTerminated = FDateTime::ParseIso8601(
		*ProcessInfoJson->GetStringField(TEXT("terminatedAt")),
		ProcessInfo.TerminatedAt
	);

	ProcessInfo.bStopping = FDateTime::ParseIso8601(
		*ProcessInfoJson->GetStringField(TEXT("stoppingAt")),
		ProcessInfo.StoppingAt
	);

	ProcessInfo.bStarted = FDateTime::ParseIso8601(
		*ProcessInfoJson->GetStringField(TEXT("startedAt")),
		ProcessInfo.StartedAt
	);

	FDateTime::ParseIso8601(
		*ProcessInfoJson->GetStringField(TEXT("startingAt")),
		ProcessInfo.StartingAt
	);

	ProcessInfo.RoomsPerProcess = ProcessInfoJson->GetIntegerField(TEXT("roomsPerProcess"));

	TArray<TSharedPtr<FJsonValue>> AdditionalExposedPorts = ProcessInfoJson->GetArrayField(TEXT("additionalExposedPorts"));
	FJsonObjectConverter::JsonArrayToUStruct(AdditionalExposedPorts, &ProcessInfo.AdditionalExposedPorts, 0, 0);

	TSharedPtr<FJsonObject> ExposedPort = ProcessInfoJson->GetObjectField(TEXT("exposedPort"));
	if (ExposedPort.IsValid())
	{
		FJsonObjectConverter::JsonObjectToUStruct(ExposedPort.ToSharedRef(), &ProcessInfo.ExposedPort, 0, 0);
	}

	ProcessInfo.Region = ParseRegion(ProcessInfoJson->GetStringField(TEXT("region")));
	ProcessInfo.ProcessId = ProcessInfoJson->GetStringField(TEXT("processId"));
	ProcessInfo.DeploymentId = ProcessInfoJson->GetIntegerField(TEXT("deploymentId"));
	ProcessInfo.AppId = ProcessInfoJson->GetStringField(TEXT("appId"));

	return ProcessInfo;
}