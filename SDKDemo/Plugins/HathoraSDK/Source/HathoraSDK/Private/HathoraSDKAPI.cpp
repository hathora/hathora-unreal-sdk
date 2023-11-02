// Copyright 2023 Hathora, Inc.

#include "HathoraSDKAPI.h"
#include "HathoraSDKConfig.h"
#include "HttpModule.h"
#include "JsonObjectWrapper.h"

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

	if (Security.AuthToken.Len() > 0)
	{
		Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Security.AuthToken));
	}

	Request->ProcessRequest();
}

FString UHathoraSDKAPI::GetRegionString(EHathoraCloudRegion Region)
{
	FString RegionString = UEnum::GetValueAsString(Region);
	RegionString = RegionString.RightChop(RegionString.Find("::") + 2);

	return RegionString;
}

EHathoraCloudRegion UHathoraSDKAPI::ParseRegion(FString RegionString)
{
	if (RegionString == TEXT("Seattle"))
	{
		return EHathoraCloudRegion::Seattle;
	}
	else if (RegionString == TEXT("Washington_DC"))
	{
		return EHathoraCloudRegion::Washington_DC;
	}
	else if (RegionString == TEXT("Chicago"))
	{
		return EHathoraCloudRegion::Chicago;
	}
	else if (RegionString == TEXT("London"))
	{
		return EHathoraCloudRegion::London;
	}
	else if (RegionString == TEXT("Frankfurt"))
	{
		return EHathoraCloudRegion::Frankfurt;
	}
	else if (RegionString == TEXT("Mumbai"))
	{
		return EHathoraCloudRegion::Mumbai;
	}
	else if (RegionString == TEXT("Singapore"))
	{
		return EHathoraCloudRegion::Singapore;
	}
	else if (RegionString == TEXT("Tokyo"))
	{
		return EHathoraCloudRegion::Tokyo;
	}
	else if (RegionString == TEXT("Sydney"))
	{
		return EHathoraCloudRegion::Sydney;
	}
	else if (RegionString == TEXT("Sao_Paulo"))
	{
		return EHathoraCloudRegion::Sao_Paulo;
	}
	else
	{
		return EHathoraCloudRegion::Unknown;
	}
}
