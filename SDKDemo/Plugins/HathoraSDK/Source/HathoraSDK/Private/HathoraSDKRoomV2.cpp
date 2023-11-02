// Copyright 2023 Hathora, Inc.

#include "HathoraSDKRoomV2.h"
#include "HathoraSDKModule.h"
#include "JsonUtilities.h"
#include "Serialization/JsonSerializer.h"

void UHathoraSDKRoomV2::CreateRoom(EHathoraCloudRegion Region, FString RoomConfig, FString RoomId, FHathoraOnCreateRoom OnComplete)
{
	FString RegionString = GetRegionString(Region);

	TArray<TPair<FString, FString>> QueryOptions;
	if (RoomId.Len() > 0)
	{
		QueryOptions.Add(TPair<FString, FString>(TEXT("roomId"), RoomId));
	}

	FJsonObject Body;
	Body.SetStringField(TEXT("region"), RegionString);
	if (RoomConfig.Len() > 0)
	{
		Body.SetStringField(TEXT("roomConfig"), RoomConfig);
	}

	SendRequest(
		TEXT("POST"),
		FString::Printf(TEXT("/rooms/v2/%s/create"), *AppId),
		QueryOptions,
		Body,
		[&, OnComplete](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) mutable
		{
			FHathoraCreateRoomResult Result;
			if (bSuccess && Response.IsValid())
			{
				Result.StatusCode = Response->GetResponseCode();
				FString Content = Response->GetContentAsString();

				if (Result.StatusCode == 201)
				{
					FJsonObjectConverter::JsonObjectStringToUStruct(Content, &Result.Data, 0, 0);
				}
				else
				{
					Result.ErrorMessage = Content;
				}
			}
			else
			{
				Result.ErrorMessage = TEXT("Could not create room, unknown error");
			}

			if (!Result.ErrorMessage.IsEmpty())
			{
				UE_LOG(LogHathoraSDK, Error, TEXT("[CreateRoom] Error: %s"), *Result.ErrorMessage);
			}

			if (!OnComplete.ExecuteIfBound(Result))
			{
				UE_LOG(LogHathoraSDK, Warning, TEXT("[CreateRoom] function pointer was not valid, so OnComplete will not be called"));
			}
		});
}

FHathoraAllocation UHathoraSDKRoomV2::ParseAllocation(const TSharedPtr<FJsonObject>& AllocationJson)
{
	FHathoraAllocation Allocation;

	FString UnscheduledAt;
	AllocationJson->TryGetStringField(TEXT("unscheduledAt"), UnscheduledAt);

	Allocation.bUnscheduled = UnscheduledAt.Len() > 0;
	if (Allocation.bUnscheduled)
	{
		FDateTime::ParseIso8601(*UnscheduledAt, Allocation.UnscheduledAt);
	}

	FDateTime::ParseIso8601(
		*AllocationJson->GetStringField(TEXT("scheduledAt")),
		Allocation.ScheduledAt
	);

	Allocation.ProcessId = AllocationJson->GetStringField(TEXT("processId"));

	Allocation.RoomAllocationId = AllocationJson->GetStringField(TEXT("roomAllocationId"));

	return Allocation;
}

EHathoraRoomStatus UHathoraSDKRoomV2::ParseRoomStatus(const FString& Status)
{
	if (Status == TEXT("scheduling"))
	{
		return EHathoraRoomStatus::Scheduling;
	}
	else if (Status == TEXT("active"))
	{
		return EHathoraRoomStatus::Active;
	}
	else if (Status == TEXT("suspended"))
	{
		return EHathoraRoomStatus::Suspended;
	}
	else if (Status == TEXT("destroyed"))
	{
		return EHathoraRoomStatus::Destroyed;
	}
	else
	{
		return EHathoraRoomStatus::Unknown;
	}
}

void UHathoraSDKRoomV2::GetRoomInfo(FString RoomId, FHathoraOnGetRoomInfo OnComplete)
{
	SendRequest(
		TEXT("GET"),
		FString::Printf(TEXT("/rooms/v2/%s/info/%s"), *AppId, *RoomId),
		[&, OnComplete](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) mutable
		{
			FHathoraGetRoomInfoResult Result;
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
						TSharedPtr<FJsonValue> CurrentAllocation = OutObject->TryGetField(TEXT("currentAllocation"));
						if (CurrentAllocation.IsValid() && !CurrentAllocation->IsNull())
						{
							Result.Data.CurrentAllocation = ParseAllocation(CurrentAllocation->AsObject());
						}

						Result.Data.Status = ParseRoomStatus(OutObject->GetStringField(TEXT("status")));

						TArray<TSharedPtr<FJsonValue>> Allocations = OutObject->GetArrayField(TEXT("allocations"));
						for (TSharedPtr<FJsonValue> Allocation : Allocations)
						{
							Result.Data.Allocations.Add(ParseAllocation(Allocation->AsObject()));
						}

						// roomConfig can be null; this will keep it empty if it is
						OutObject->TryGetStringField(TEXT("roomConfig"), Result.Data.RoomConfig);

						Result.Data.RoomId = OutObject->GetStringField(TEXT("roomId"));
						Result.Data.AppId = OutObject->GetStringField(TEXT("appId"));
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
				Result.ErrorMessage = TEXT("Could not get room info, unknown error");
			}

			if (!Result.ErrorMessage.IsEmpty())
			{
				UE_LOG(LogHathoraSDK, Error, TEXT("[GetRoomInfo] Error: %s"), *Result.ErrorMessage);
			}

			if (!OnComplete.ExecuteIfBound(Result))
			{
				UE_LOG(LogHathoraSDK, Warning, TEXT("[GetRoomInfo] function pointer was not valid, so OnComplete will not be called"));
			}
		});
}