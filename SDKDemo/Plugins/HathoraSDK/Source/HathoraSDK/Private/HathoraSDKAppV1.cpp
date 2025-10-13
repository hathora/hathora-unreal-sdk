// Copyright 2023 Hathora, Inc.

#include "HathoraSDKAppV1.h"
#include "HathoraSDKModule.h"
#include "HathoraSDK.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Misc/DateTime.h"

FHathoraApplication UHathoraSDKAppV1::ParseApplication(TSharedPtr<FJsonObject> Object)
{
	FHathoraApplication Application;

	TSharedPtr<FJsonValue> DeletedBy = Object->TryGetField(TEXT("deletedBy"));

	Application.bDeleted = DeletedBy.IsValid() && !DeletedBy->IsNull();

	if (Application.bDeleted)
	{
		Application.DeletedBy = DeletedBy->AsString();

		FDateTime::ParseIso8601(
			*Object->GetStringField(TEXT("deletedAt")),
			Application.DeletedAt
		);
	}

	FDateTime::ParseIso8601(
		*Object->GetStringField(TEXT("createdAt")),
		Application.CreatedAt
	);

	Application.CreatedBy = Object->GetStringField(TEXT("createdBy"));
	Application.OrgId = Object->GetStringField(TEXT("orgId"));

	Application.AuthConfiguration.bGoogleEnabled = Object->HasField(TEXT("google"));
	Application.AuthConfiguration.bNicknameEnabled = Object->HasField(TEXT("nickname"));
	Application.AuthConfiguration.bAnonymousEnabled = Object->HasField(TEXT("anonymous"));

	if (Application.AuthConfiguration.bGoogleEnabled)
	{
		TSharedPtr<FJsonObject> Google = Object->GetObjectField(TEXT("google"));
		Application.AuthConfiguration.Google.ClientId = Google->GetStringField(TEXT("clientId"));
	}

	Application.AppSecret = Object->GetStringField(TEXT("appSecret"));
	Application.AppId = Object->GetStringField(TEXT("appId"));
	Application.AppName = Object->GetStringField(TEXT("appName"));

	// TODO: deployment?

	return Application;
}

void UHathoraSDKAppV1::GetApps(FHathoraOnGetApps OnComplete)
{
	SendRequest(
		TEXT("GET"),
		TEXT("/apps/v1/list"),
		[OnComplete](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) mutable
		{
			FHathoraGetAppsResult Result;
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
						Result.Data.Add(ParseApplication(Object));
					}
				}
				else
				{
					Result.ErrorMessage = UHathoraSDK::ParseErrorMessage(Content);
				}
			}
			else
			{
				Result.ErrorMessage = TEXT("[GetApps] Error: unknown error");
			}

			if (!Result.ErrorMessage.IsEmpty())
			{
				UE_LOG(LogHathoraSDK, Error, TEXT("[GetApps] Error: %s"), *Result.ErrorMessage);
			}

			if (!OnComplete.ExecuteIfBound(Result))
			{
				UE_LOG(LogHathoraSDK, Warning, TEXT("[GetApps] function pointer was not valid, so OnComplete will not be called"));
			}
		}
	);
}
