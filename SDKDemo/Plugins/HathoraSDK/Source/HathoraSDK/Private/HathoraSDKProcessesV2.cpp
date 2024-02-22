// Copyright 2023 Hathora, Inc.

#include "HathoraSDKProcessesV2.h"
#include "HathoraSDKModule.h"
#include "HathoraSDK.h"
#include "Serialization/JsonSerializer.h"
#include "JsonObjectConverter.h"

FString UHathoraSDKProcessesV2::GetProcessStatusString(EHathoraProcessStatus Status)
{
	FString StatusString = UEnum::GetValueAsString(Status);
	StatusString = StatusString.ToLower().RightChop(StatusString.Find("::") + 2);

	return StatusString;
}

EHathoraProcessStatus UHathoraSDKProcessesV2::ParseStatus(FString StatusString)
{
	EHathoraProcessStatus Status = EHathoraProcessStatus::Unknown;
	for (uint32 i = 0; i < static_cast<uint8>(EHathoraProcessStatus::Unknown); i++)
	{
		FString CurrentStatus = GetProcessStatusString(static_cast<EHathoraProcessStatus>(i));
		if (CurrentStatus == StatusString)
		{
			Status = static_cast<EHathoraProcessStatus>(i);
			break;
		}
	}

	if (Status == EHathoraProcessStatus::Unknown)
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("[ParseRegion] Unknown status: %s"), *StatusString);
	}

	return Status;
}

void UHathoraSDKProcessesV2::GetLatestProcesses(TArray<EHathoraProcessStatus> StatusFilter, TArray<EHathoraCloudRegion> RegionFilter, FHathoraOnProcessInfos OnComplete)
{
	TArray<TPair<FString, FString>> QueryOptions;

	for (EHathoraProcessStatus Status : StatusFilter)
	{
		QueryOptions.Add(TPair<FString, FString>(TEXT("status"), UHathoraSDKProcessesV2::GetProcessStatusString(Status)));
	}

	for (EHathoraCloudRegion Region : RegionFilter)
	{
		QueryOptions.Add(TPair<FString, FString>(TEXT("region"), UHathoraSDK::GetRegionString(Region)));
	}

	SendRequest(
		TEXT("GET"),
		FString::Printf(TEXT("/processes/v2/%s/list/latest"), *AppId),
		QueryOptions,
		[OnComplete](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) mutable
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
					Result.ErrorMessage = UHathoraSDK::ParseErrorMessage(Content);
				}
			}
			else
			{
				Result.ErrorMessage = TEXT("Could not list processes, unknown error");
			}

			if (!Result.ErrorMessage.IsEmpty())
			{
				UE_LOG(LogHathoraSDK, Error, TEXT("[GetLatestProcesses] Error: %s"), *Result.ErrorMessage);
			}

			if (!OnComplete.ExecuteIfBound(Result))
			{
				UE_LOG(LogHathoraSDK, Warning, TEXT("[GetLatestProcesses] function pointer was not valid, so OnComplete will not be called"));
			}
		});
}

void UHathoraSDKProcessesV2::GetProcessInfo(FString ProcessId, FHathoraOnProcessInfo OnComplete)
{
	SendRequest(
		TEXT("GET"),
		FString::Printf(TEXT("/processes/v2/%s/info/%s"), *AppId, *ProcessId),
		[OnComplete](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) mutable
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
					Result.ErrorMessage = UHathoraSDK::ParseErrorMessage(Content);
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

FHathoraProcessInfo UHathoraSDKProcessesV2::ParseProcessInfo(TSharedPtr<FJsonObject> ProcessInfoJson)
{
	FHathoraProcessInfo ProcessInfo;

	ProcessInfo.Status = ParseStatus(ProcessInfoJson->GetStringField(TEXT("status")));

	ProcessInfo.RoomsAllocated = ProcessInfoJson->GetIntegerField(TEXT("roomsAllocated"));

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
		*ProcessInfoJson->GetStringField(TEXT("createdAt")),
		ProcessInfo.CreatedAt
	);

	ProcessInfo.RoomsPerProcess = ProcessInfoJson->GetIntegerField(TEXT("roomsPerProcess"));

	TArray<TSharedPtr<FJsonValue>> AdditionalExposedPorts = ProcessInfoJson->GetArrayField(TEXT("additionalExposedPorts"));
	FJsonObjectConverter::JsonArrayToUStruct(AdditionalExposedPorts, &ProcessInfo.AdditionalExposedPorts, 0, 0);

	TSharedPtr<FJsonObject> ExposedPort = ProcessInfoJson->GetObjectField(TEXT("exposedPort"));
	if (ExposedPort.IsValid())
	{
		FJsonObjectConverter::JsonObjectToUStruct(ExposedPort.ToSharedRef(), &ProcessInfo.ExposedPort, 0, 0);
	}

	ProcessInfo.Region = UHathoraSDK::ParseRegion(ProcessInfoJson->GetStringField(TEXT("region")));
	ProcessInfo.ProcessId = ProcessInfoJson->GetStringField(TEXT("processId"));
	ProcessInfo.DeploymentId = ProcessInfoJson->GetIntegerField(TEXT("deploymentId"));
	ProcessInfo.AppId = ProcessInfoJson->GetStringField(TEXT("appId"));

	return ProcessInfo;
}

void UHathoraSDKProcessesV2::StopProcess(FString ProcessId, FHathoraOnStopProcess OnComplete)
{
	SendRequest(
		TEXT("POST"),
		FString::Printf(TEXT("/processes/v2/%s/stop/%s"), *AppId, *ProcessId),
		[OnComplete](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) mutable
		{
			FHathoraStopProcessResult Result;
			if (bSuccess && Response.IsValid())
			{
				Result.StatusCode = Response->GetResponseCode();
				FString Content = Response->GetContentAsString();

				if (Result.StatusCode != 204)
				{
					Result.ErrorMessage = UHathoraSDK::ParseErrorMessage(Content);
				}
			}
			else
			{
				Result.ErrorMessage = TEXT("Could not stop process unknown error");
			}

			if (!Result.ErrorMessage.IsEmpty())
			{
				UE_LOG(LogHathoraSDK, Error, TEXT("[StopProcess] Error: %s"), *Result.ErrorMessage);
			}

			if (!OnComplete.ExecuteIfBound(Result))
			{
				UE_LOG(LogHathoraSDK, Warning, TEXT("[StopProcess] function pointer was not valid, so OnComplete will not be called"));
			}
		});
}
