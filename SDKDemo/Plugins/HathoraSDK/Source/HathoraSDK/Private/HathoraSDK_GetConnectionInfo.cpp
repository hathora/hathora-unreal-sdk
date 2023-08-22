// Copyright 2023 Hathora, Inc. All Rights Reserved.

#include "HathoraSDK.h"

#include "TimerManager.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

#define LOCTEXT_NAMESPACE "FHathoraSDKModule"

void FHathoraSDKModule::GetConnectionInfo(const FString& AppId, const FString& RoomId, const FConnectionInfoDelegate& OnComplete)
{
	GetConnectionInfo_Internal(AppId, RoomId, FPlatformTime::Seconds(), 0, OnComplete);
}

void FHathoraSDKModule::GetConnectionInfo_Internal(const FString& AppId, const FString& RoomId, double StartTime, int32 RetryCount, const FConnectionInfoDelegate& OnComplete)
{
	TSharedRef< IHttpRequest, ESPMode::ThreadSafe > request = NewRequest();
	request->SetURL(FString::Format(TEXT("{0}/rooms/v2/{1}/connectioninfo/{2}"), { BaseUrl, AppId, RoomId }));

	request->OnProcessRequestComplete().BindLambda([this, AppId, RoomId, StartTime, RetryCount, OnComplete](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) {
		if (!bSuccess || !Response.IsValid())
		{
			UE_LOG(LogHathoraSDK, Warning, TEXT("rooms/v2/connectioninfo failed to connect"));
			OnComplete.ExecuteIfBound(false, FString(), 0);
			return;
		}

		if (Response->GetResponseCode() != 200)
		{
			UE_LOG(LogHathoraSDK, Warning,
				TEXT("rooms/v2/connectioninfo failed AppId=\"%s\" RoomId=\"%s\" ResponseCode=%d Response=\"%s\""),
				*AppId, *RoomId, Response->GetResponseCode(), *Response->GetContentAsString());
			OnComplete.ExecuteIfBound(false, FString(), 0);
			return;
		}

		TSharedPtr<FJsonObject> jsonResponseBody;
		TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Response->GetContentAsString());
		if (!FJsonSerializer::Deserialize(JsonReader, jsonResponseBody))
		{
			UE_LOG(LogHathoraSDK, Error, TEXT("rooms/v2/connectioninfo failed to deserialize json AppId=\"%s\" RoomId=\"%s\" Response=\"%s\""),
				*AppId, *RoomId, *Response->GetContentAsString());
			OnComplete.ExecuteIfBound(false, FString(), 0);
			return;
		}

		FString status;
		if (!jsonResponseBody->TryGetStringField("status", status))
		{
			UE_LOG(LogHathoraSDK, Error, TEXT("rooms/v2/connectioninfo response missing status AppId=\"%s\" RoomId=\"%s\" Response=\"%s\""),
				*AppId, *RoomId, *Response->GetContentAsString());
			OnComplete.ExecuteIfBound(false, FString(), 0);
			return;
		}

		if (status == TEXT("starting")) {
			if ((FPlatformTime::Seconds() - StartTime) <= GetConnectionInfoTimeoutSeconds)
			{
				// Retry
				double delay = 1.0f;
				if (GetConnectionInfoDelaysSeconds.Num() > 0)
				{
					delay = FCString::Atof(*GetConnectionInfoDelaysSeconds[FGenericPlatformMath::Min(RetryCount, GetConnectionInfoDelaysSeconds.Num() - 1)]);
				}

				auto retry = [this, AppId, RoomId, StartTime, RetryCount, OnComplete]() {
					GetConnectionInfo_Internal(AppId, RoomId, StartTime, RetryCount + 1, OnComplete);
				};

				FTimerHandle unusedHandle;
				GWorld->GetTimerManager().SetTimer(unusedHandle, retry, delay, false);
				return;
			}
			else
			{
				// Timeout
				UE_LOG(LogHathoraSDK, Warning, TEXT("rooms/v2/connectioninfo timed out waiting for room to be active AppId=\"%s\" RoomId=\"%s\""),
					*AppId, *RoomId, *Response->GetContentAsString());
				OnComplete.ExecuteIfBound(false, FString(), 0);
				return;
			}
		}
		else if (status != TEXT("active")) {
			UE_LOG(LogHathoraSDK, Error, TEXT("rooms/v2/connectioninfo response has invalid status AppId=\"%s\" RoomId=\"%s\" Response=\"%s\""),
				*AppId, *RoomId, *Response->GetContentAsString());
			OnComplete.ExecuteIfBound(false, FString(), 0);
			return;
		}

		const TSharedPtr<FJsonObject>* jsonExposedPort;
		if (!jsonResponseBody->TryGetObjectField("exposedPort", jsonExposedPort))
		{
			UE_LOG(LogHathoraSDK, Error, TEXT("rooms/v2/connectioninfo response missing exposedPort AppId=\"%s\" RoomId=\"%s\" Response=\"%s\""),
				*AppId, *RoomId, *Response->GetContentAsString());
			OnComplete.ExecuteIfBound(false, FString(), 0);
			return;
		}

		FString host;
		if (!jsonExposedPort->Get()->TryGetStringField("host", host))
		{
			UE_LOG(LogHathoraSDK, Error, TEXT("rooms/v2/connectioninfo response missing host AppId=\"%s\" RoomId=\"%s\" Response=\"%s\""),
				*AppId, *RoomId, *Response->GetContentAsString());
			OnComplete.ExecuteIfBound(false, FString(), 0);
			return;
		}

		int32 port = 0;
		if (!jsonExposedPort->Get()->TryGetNumberField("port", port))
		{
			UE_LOG(LogHathoraSDK, Error, TEXT("rooms/v2/connectioninfo response missing port AppId=\"%s\" RoomId=\"%s\" Response=\"%s\""),
				*AppId, *RoomId, *Response->GetContentAsString());
			OnComplete.ExecuteIfBound(false, FString(), 0);
			return;
		}

		// Success
		OnComplete.ExecuteIfBound(true, host, port);
		});

	request->ProcessRequest();
}

#undef LOCTEXT_NAMESPACE
