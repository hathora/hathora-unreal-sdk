// Copyright 2023 Hathora, Inc.

#include "HathoraSDKAPI.h"
#include "HathoraSDKConfig.h"
#include "HathoraSDKModule.h"
#include "HttpModule.h"
#include "JsonObjectWrapper.h"
#include "Engine/World.h"

UHathoraSDKAPI::UHathoraSDKAPI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FWorldDelegates::PreLevelRemovedFromWorld.AddUObject(this, &UHathoraSDKAPI::OnWorldRemoved);
}

void UHathoraSDKAPI::SetCredentials(FString InAppId, FHathoraSDKSecurity InSecurity)
{
	this->AppId = InAppId;
	this->Security = InSecurity;
}

void UHathoraSDKAPI::SendRequest(
	FString Method,
	FString Endpoint,
	TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> OnProcessRequestComplete)
{
	SendRequest(Method, Endpoint, TArray<TPair<FString, FString>>(), FJsonObject(), OnProcessRequestComplete);
}

void UHathoraSDKAPI::SendRequest(
	FString Method,
	FString Endpoint,
	TArray<TPair<FString, FString>> QueryOptions,
	TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> OnProcessRequestComplete)
{
	SendRequest(Method, Endpoint, QueryOptions, FJsonObject(), OnProcessRequestComplete);
}

void UHathoraSDKAPI::SendRequest(
	FString Method,
	FString Endpoint,
	FJsonObject Body,
	TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> OnProcessRequestComplete)
{
	SendRequest(Method, Endpoint, TArray<TPair<FString, FString>>(), Body, OnProcessRequestComplete);
}

void UHathoraSDKAPI::SendRequest(
	FString Method,
	FString Endpoint,
	TArray<TPair<FString, FString>> QueryOptions,
	FJsonObject Body,
	TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> OnComplete)
{
	if (bWorldIsBeingDestroyed)
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("Could not make %s request to %s because the world is being destroyed."), *Method, *Endpoint);
		return;
	}

	if (!IsValid(this))
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("Could not make %s request to %s because the underlying Hathora API object is not valid."), *Method, *Endpoint);
		return;
	}

	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();

	Request->SetVerb(Method);

	Request->OnProcessRequestComplete().BindLambda(
		[this, Method, Endpoint, OnComplete](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
		{
			if (bWorldIsBeingDestroyed)
			{
				UE_LOG(LogHathoraSDK, Error, TEXT("Could not call OnComplete for %s request to %s because the world is being destroyed."), *Method, *Endpoint);
				return;
			}

			OnComplete(Request, Response, bWasSuccessful);
		}
	);

	FString QueryString = "?";
	for (auto QueryOption : QueryOptions) {
		QueryString += QueryOption.Key + "=" + QueryOption.Value + "&";
	}
	// remove the last &, or ? if there are no options
	QueryString = QueryString.LeftChop(1);

	Request->SetURL(
		FString::Printf(
			TEXT("%s%s%s"),
			*GetDefault<UHathoraSDKConfig>()->GetBaseUrl(),
			*Endpoint,
			*QueryString
		)
	);

	if (!Body.Values.IsEmpty())
	{
		FString JsonString;
		FJsonObjectWrapper JsonObjectWrapper;
		JsonObjectWrapper.JsonObject = MakeShareable(new FJsonObject(Body));
		JsonObjectWrapper.JsonObjectToString(JsonString);
		Request->SetContentAsString(*JsonString);
	}

	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	if (Security.AuthToken.Len() > 0)
	{
		Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Security.AuthToken));
	}

	Request->ProcessRequest();
}

void UHathoraSDKAPI::OnWorldRemoved(ULevel* Level, UWorld* World)
{
	bWorldIsBeingDestroyed = true;
	FWorldDelegates::PreLevelRemovedFromWorld.RemoveAll(this);
}
