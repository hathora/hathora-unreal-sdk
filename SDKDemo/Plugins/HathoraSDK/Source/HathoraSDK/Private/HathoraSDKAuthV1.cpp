// Copyright 2023 Hathora, Inc.

#include "HathoraSDKAuthV1.h"
#include "HathoraSDKModule.h"

void UHathoraSDKAuthV1::LoginAnonymous(FHathoraOnLogin OnComplete)
{
	Login(
		FString::Printf(TEXT("/auth/v1/%s/login/anonymous"), *AppId),
		FJsonObject(),
		OnComplete
	);
}

void UHathoraSDKAuthV1::LoginNickname(FString Nickname, FHathoraOnLogin OnComplete)
{
	FJsonObject Body;
	Body.SetStringField(TEXT("nickname"), Nickname);

	Login(
		FString::Printf(TEXT("/auth/v1/%s/login/nickname"), *AppId),
		Body,
		OnComplete
	);
}

void UHathoraSDKAuthV1::LoginGoogle(FString IdToken, FHathoraOnLogin OnComplete)
{
	FJsonObject Body;
	Body.SetStringField(TEXT("idToken"), IdToken);

	Login(
		FString::Printf(TEXT("/auth/v1/%s/login/google"), *AppId),
		Body,
		OnComplete
	);
}

void UHathoraSDKAuthV1::Login(FString Path, FJsonObject Body, FHathoraOnLogin OnComplete)
{
	SendRequest(
		TEXT("POST"),
		Path,
		Body,
		[&, OnComplete](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) mutable
		{
			FHathoraLoginResult Result;
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
						Result.Token = OutObject->GetStringField(TEXT("token"));
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
				Result.ErrorMessage = TEXT("Could not login, unknown error");
			}

			if (!Result.ErrorMessage.IsEmpty())
			{
				UE_LOG(LogHathoraSDK, Error, TEXT("[LoginAnonymous] Error: %s"), *Result.ErrorMessage);
			}

			if (!OnComplete.ExecuteIfBound(Result))
			{
				UE_LOG(LogHathoraSDK, Warning, TEXT("[LoginAnonymous] function pointer was not valid, so OnComplete will not be called"));
			}
		}
	);
}