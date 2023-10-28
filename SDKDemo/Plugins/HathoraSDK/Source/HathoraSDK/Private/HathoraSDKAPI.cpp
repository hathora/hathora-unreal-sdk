// Copyright 2023 Hathora, Inc.

#include "HathoraSDKAPI.h"
#include "HathoraSDKConfig.h"
#include "HttpModule.h"
#include "JsonUtilities.h"

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
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();

	Request->SetVerb(Method);

	Request->OnProcessRequestComplete().BindLambda(OnComplete);

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

	if (Security.HathoraDevToken.Len() > 0)
	{
		Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Security.HathoraDevToken));
	}

	Request->ProcessRequest();
}

FString UHathoraSDKAPI::GetRegionString(EHathoraCloudRegion Region)
{
	FString RegionString = UEnum::GetValueAsString(Region);
	RegionString = RegionString.RightChop(RegionString.Find("::") + 2);

	return RegionString;
}
