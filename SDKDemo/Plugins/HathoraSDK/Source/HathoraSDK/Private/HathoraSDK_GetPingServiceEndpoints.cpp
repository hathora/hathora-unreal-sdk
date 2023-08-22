// Copyright 2023 Hathora, Inc. All Rights Reserved.

#include "HathoraSDK.h"

#include "JsonObjectConverter.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

#define LOCTEXT_NAMESPACE "FHathoraSDKModule"

void FHathoraSDKModule::GetPingServiceEndpoints(const FPingServiceEndpointsDelegate& OnComplete)
{
	TSharedRef< IHttpRequest, ESPMode::ThreadSafe > request = NewRequest();
	request->SetURL(FString::Format(TEXT("{0}/discovery/v1/ping"), { BaseUrl }));
	request->OnProcessRequestComplete().BindLambda([OnComplete](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) {
		TArray<FDiscoveredPingEndpoint> endpoints;
		if (bSuccess && Response.IsValid() && FJsonObjectConverter::JsonArrayStringToUStruct(Response->GetContentAsString(), &endpoints))
		{
			if (endpoints.Num() > 0)
			{
				UE_LOG(LogHathoraSDK, Warning, TEXT("/discovery/v1/ping returned an empty list of regions"));
			}

			OnComplete.ExecuteIfBound(endpoints);
		}
		else
		{
			UE_LOG(LogHathoraSDK, Warning, TEXT("Could not retrieve ping endpoints"))
			TArray<FDiscoveredPingEndpoint> emptyResult;
			OnComplete.ExecuteIfBound(emptyResult);
		}
	});
	request->ProcessRequest();
}

#undef LOCTEXT_NAMESPACE
