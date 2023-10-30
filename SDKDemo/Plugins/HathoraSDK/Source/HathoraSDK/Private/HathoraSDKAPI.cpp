// Copyright 2023 Hathora, Inc.

#include "HathoraSDKAPI.h"
#include "HathoraSDKConfig.h"
#include "HttpModule.h"

void UHathoraSDKAPI::SetCredentials(FString InAppId, FHathoraSDKSecurity InSecurity)
{
	this->AppId = AppId;
	this->Security = Security;
}

void UHathoraSDKAPI::SendRequest(FString Method, FString Endpoint, TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> OnComplete)
{
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();

	Request->SetVerb(Method);

	Request->OnProcessRequestComplete().BindLambda(OnComplete);

	Request->SetURL(FString::Printf(TEXT("%s%s"), *GetDefault<UHathoraSDKConfig>()->GetBaseUrl(), *Endpoint));
	Request->ProcessRequest();
}
