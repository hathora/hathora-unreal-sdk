// Copyright 2023 Hathora, Inc.

#include "HathoraSDK.h"
#include "HathoraSDKModule.h"
#include "HathoraSDKConfig.h"
#include "HathoraSDKAuthV1.h"
#include "HathoraSDKDiscoveryV2.h"
#include "HathoraSDKLobbyV3.h"
#include "HathoraSDKProcessesV2.h"
#include "HathoraSDKRoomV2.h"
#include "HathoraPingUtility.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"

void UHathoraSDK::GetRegionalPings(const FHathoraOnGetRegionalPings& OnComplete, int32 NumPingsPerRegion)
{
	Internal_GetRegionalPings(OnComplete, NumPingsPerRegion);
}

void UHathoraSDK::Internal_GetRegionalPings(const FHathoraOnGetRegionalPings& OnComplete, int32 NumPingsPerRegion)
{
	UHathoraSDK::GetPingsForRegions(
		UHathoraSDK::GetRegionMap(),
		EHathoraPingType::ICMP,
		OnComplete,
		NumPingsPerRegion
	);
}

void UHathoraSDK::GetPingsForRegions(TMap<FString, FString> Regions, EHathoraPingType PingType, const FHathoraOnGetRegionalPings& OnComplete, int32 NumPingsPerRegion)
{
	UHathoraPingUtility* PingUtility = NewObject<UHathoraPingUtility>();
	PingUtility->AddToRoot(); // make sure this doesn't get garbage collected
	PingUtility->GetPingsForRegions(
		Regions,
		PingType,
		FHathoraOnGetRegionalPings::CreateLambda(
			[OnComplete, PingUtility](const FHathoraRegionPings& Result)
			{
				OnComplete.ExecuteIfBound(Result);
				PingUtility->RemoveFromRoot(); // Allow this SDK reference to be garbage collected
			}
		),
		NumPingsPerRegion
	);
}

TMap<FString, FString> UHathoraSDK::GetRegionMap()
{
	TMap<FString, FString> RegionMap;

	RegionMap.Add(TEXT("Chicago"), TEXT("chicago.ping.hathora.dev:10000"));
	RegionMap.Add(TEXT("Dallas"), TEXT("dallas.ping.hathora.dev:10000"));
	RegionMap.Add(TEXT("Dubai"), TEXT("dubai.ping.hathora.dev:10000"));
	RegionMap.Add(TEXT("Frankfurt"), TEXT("frankfurt.ping.hathora.dev:10000"));
	RegionMap.Add(TEXT("Johannesburg"), TEXT("johannesburg.ping.hathora.dev:10000"));
	RegionMap.Add(TEXT("London"), TEXT("london.ping.hathora.dev:10000"));
	RegionMap.Add(TEXT("Los Angeles"), TEXT("losangeles.ping.hathora.dev:10000"));
	RegionMap.Add(TEXT("Mumbai"), TEXT("mumbai.ping.hathora.dev:10000"));
	RegionMap.Add(TEXT("Sao Paulo"), TEXT("saopaulo.ping.hathora.dev:10000"));
	RegionMap.Add(TEXT("Seattle"), TEXT("seattle.ping.hathora.dev:10000"));
	RegionMap.Add(TEXT("Singapore"), TEXT("singapore.ping.hathora.dev:10000"));
	RegionMap.Add(TEXT("Sydney"), TEXT("sydney.ping.hathora.dev:10000"));
	RegionMap.Add(TEXT("Tokyo"), TEXT("tokyo.ping.hathora.dev:10000"));
	RegionMap.Add(TEXT("Washington DC"), TEXT("washingtondc.ping.hathora.dev:10000"));

	return RegionMap;
}

UHathoraSDK* UHathoraSDK::CreateHathoraSDK()
{
	UHathoraSDK* SDK = NewObject<UHathoraSDK>();
	SDK->AuthV1 = NewObject<UHathoraSDKAuthV1>();
	SDK->DiscoveryV2 = NewObject<UHathoraSDKDiscoveryV2>();
	SDK->LobbyV3 = NewObject<UHathoraSDKLobbyV3>();
	SDK->ProcessesV2 = NewObject<UHathoraSDKProcessesV2>();
	SDK->RoomV2 = NewObject<UHathoraSDKRoomV2>();

	const UHathoraSDKConfig* Config = GetDefault<UHathoraSDKConfig>();
	FString AppId = Config->GetAppId();
	FHathoraSDKSecurity Security(Config->GetDevToken());

	SDK->SetCredentials(Config->GetAppId(), Security);

	return SDK;
}

FHathoraServerEnvironment UHathoraSDK::GetServerEnvironment()
{
	FHathoraServerEnvironment Environment;

	Environment.AppId = FPlatformMisc::GetEnvironmentVariable(TEXT("HATHORA_APP_ID"));
	Environment.AppSecret = FPlatformMisc::GetEnvironmentVariable(TEXT("HATHORA_APP_SECRET"));
	Environment.ProcessId = FPlatformMisc::GetEnvironmentVariable(TEXT("HATHORA_PROCESS_ID"));
	Environment.DeploymentId = FPlatformMisc::GetEnvironmentVariable(TEXT("HATHORA_DEPLOYMENT_ID"));
	Environment.BuildTag = FPlatformMisc::GetEnvironmentVariable(TEXT("HATHORA_BUILD_TAG"));
	Environment.Region = ParseRegion(FPlatformMisc::GetEnvironmentVariable(TEXT("HATHORA_REGION")));
	Environment.RoomsPerProcess = FCString::Atoi(*FPlatformMisc::GetEnvironmentVariable(TEXT("HATHORA_ROOMS_PER_PROCESS")));
	Environment.InitialRoomId = FPlatformMisc::GetEnvironmentVariable(TEXT("HATHORA_INITIAL_ROOM_ID"));
	Environment.InitialRoomConfig = FPlatformMisc::GetEnvironmentVariable(TEXT("HATHORA_INITIAL_ROOM_CONFIG"));
	Environment.Hostname = FPlatformMisc::GetEnvironmentVariable(TEXT("HATHORA_HOSTNAME"));
	Environment.DefaultPort = FCString::Atoi(*FPlatformMisc::GetEnvironmentVariable(TEXT("HATHORA_DEFAULT_PORT")));

	return Environment;
}

FString UHathoraSDK::GetRegionString(EHathoraCloudRegion Region)
{
	FString RegionString = UEnum::GetValueAsString(Region);
	RegionString = RegionString.RightChop(RegionString.Find("::") + 2);

	return RegionString;
}

EHathoraCloudRegion UHathoraSDK::ParseRegion(FString RegionString)
{
	EHathoraCloudRegion Region = EHathoraCloudRegion::Unknown;
	for (uint32 i = 0; i < static_cast<uint8>(EHathoraCloudRegion::Unknown); i++)
	{
		FString CurrentRegion = UHathoraSDK::GetRegionString(static_cast<EHathoraCloudRegion>(i));
		if (CurrentRegion == RegionString)
		{
			Region = static_cast<EHathoraCloudRegion>(i);
			break;
		}
	}

	if (Region == EHathoraCloudRegion::Unknown)
	{
		UE_LOG(LogHathoraSDK, Error, TEXT("[ParseRegion] Unknown region: %s"), *RegionString);
	}

	return Region;
}

FString UHathoraSDK::ParseErrorMessage(FString Content)
{
	TSharedPtr<FJsonObject> OutObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
	FJsonSerializer::Deserialize(Reader, OutObject);

	if (OutObject.IsValid())
	{
		FString Message;
		bool bValid = OutObject->TryGetStringField(TEXT("message"), Message);

		if (bValid)
		{
			return Message;
		}
		else
		{
			return Content;
		}
	}
	else
	{
		return Content;
	}
}

void UHathoraSDK::SetAuthToken(FString Token)
{
	const UHathoraSDKConfig* Config = GetDefault<UHathoraSDKConfig>();
	FString AppId = Config->GetAppId();
	FHathoraSDKSecurity Security(Token);

	SetCredentials(Config->GetAppId(), Security);
}

bool UHathoraSDK::IsLoggedIn()
{
	return AuthV1->IsLoggedIn();
}

void UHathoraSDK::SetCredentials(FString AppId, FHathoraSDKSecurity Security)
{
	AuthV1->SetCredentials(AppId, Security);
	DiscoveryV2->SetCredentials(AppId, Security);
	LobbyV3->SetCredentials(AppId, Security);
	ProcessesV2->SetCredentials(AppId, Security);
	RoomV2->SetCredentials(AppId, Security);
}
