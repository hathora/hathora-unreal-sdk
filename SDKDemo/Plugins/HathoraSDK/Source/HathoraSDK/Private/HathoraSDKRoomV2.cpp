// Copyright 2023 Hathora, Inc.

#include "HathoraSDKRoomV2.h"
#include "HathoraSDKModule.h"
#include "JsonUtilities.h"

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
				UE_LOG(LogHathoraSDK, Error, TEXT("%s"), *Result.ErrorMessage);
			}

			if (!OnComplete.ExecuteIfBound(Result))
			{
				UE_LOG(LogHathoraSDK, Warning, TEXT("[CreateRoom] function pointer was not valid, so OnComplete will not be called"));
			}
		});
}