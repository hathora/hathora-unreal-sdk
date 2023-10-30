// Copyright 2023 Hathora, Inc.

#include "HathoraSDK.h"
#include "HathoraSDKModule.h"
#include "HathoraSDKDiscoveryV1.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

void UHathoraSDK::GetRegionalPings(const FHathoraOnGetRegionalPings& OnComplete)
{
	UHathoraSDK* SDK = UHathoraSDK::CreateHathoraSDK("", FHathoraSDKSecurity());
	SDK->DiscoveryV1->GetRegionalPings(OnComplete);
}

UHathoraSDK* UHathoraSDK::CreateHathoraSDK(FString AppId, FHathoraSDKSecurity Security)
{
	UHathoraSDK* SDK = NewObject<UHathoraSDK>();
	SDK->DiscoveryV1 = NewObject<UHathoraSDKDiscoveryV1>();

	SDK->SetCredentials(AppId, Security);

	return SDK;
}

void UHathoraSDK::SetCredentials(FString AppId, FHathoraSDKSecurity Security)
{
	DiscoveryV1->SetCredentials(AppId, Security);
}
