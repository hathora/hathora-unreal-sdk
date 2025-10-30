// Copyright 2025 Hathora, Inc.

#include "HathoraPingUtility.h"
#include "HathoraSDKModule.h"
#include "HathoraSDKConfig.h"
#include "Icmp.h"

void UHathoraPingUtility::GetPingsForRegions(TMap<FString, FString> InRegionUrls, EHathoraPingType InPingType, const FHathoraOnGetRegionalPings& OnComplete, int32 InNumPingsPerRegion)
{
	RegionUrls = InRegionUrls;
	PingType = InPingType;
	OnGetRegionalPingsComplete = OnComplete;
	NumPingsPerRegion = InNumPingsPerRegion;
	NumPingsPerRegionCompleted = 0;
	PingResults = MakeShared<TMap<FString, TArray<int32>>>();
	PingEachRegion();
}

void UHathoraPingUtility::PingEachRegion()
{
	// recursively call this function until we have pinged each region
	// the desired number of times, and then aggregate the results and return
	// this ensures subsequent pings to a particular region are done sequentially
	// instead of simultaneously
	if (NumPingsPerRegionCompleted >= NumPingsPerRegion)
	{
		UE_LOG(LogHathoraSDK, Log, TEXT("Completed multiple pings to each Hathora region; returning the averages."));

		FHathoraRegionPings FinalResult;

		for (const TPair<FString, TArray<int32>>& PingResult : *PingResults)
		{
			int32 Min = -1;
			for (int32 PingTime : PingResult.Value)
			{
				if (Min == -1 || PingTime < Min)
				{
					Min = PingTime;
				}
			}
			FinalResult.Pings.Add(PingResult.Key, Min);
		}

		OnGetRegionalPingsComplete.ExecuteIfBound(FinalResult);

		return;
	}

	NumPingsPerRegionCompleted++;

	TSharedPtr<int32> CompletedPings = MakeShared<int32>(0);
	const int32 PingsToComplete = RegionUrls.Num();

	// ping each region once
	for (const auto& RegionUrl : RegionUrls)
	{
		FString Name = RegionUrl.Key;
		bool bHasPort = RegionUrl.Value.Contains(":");
		FString Host = bHasPort ? RegionUrl.Value.Left(RegionUrl.Value.Find(":")) : RegionUrl.Value;

		if (!bHasPort && PingType == EHathoraPingType::UDPEcho)
		{
			UE_LOG(LogHathoraSDK, Warning, TEXT("Region URL %s does not contain a port but EHathoraPingType::UDPEcho was selected; falling back to ICMP PingType."), *RegionUrl.Value);
			PingType = EHathoraPingType::ICMP;
		}

		FIcmpEchoResultCallback Callback =
			[this, Name, Host, CompletedPings, PingsToComplete](FIcmpEchoResult Result)
			{
				if (Result.Status == EIcmpResponseStatus::Success)
				{
					int32 PingTime = Result.Time * 1000;
					UE_LOG(LogHathoraSDK, Log, TEXT("Ping to %s (%s) took: %d ms"), *Name, *Host, PingTime);
					TArray<int32>* RegionPings = PingResults->Find(Name);
					if (RegionPings == nullptr)
					{
						TArray<int32> NewRegionPings;
						NewRegionPings.Add(PingTime);
						PingResults->Add(Name, NewRegionPings);
					}
					else
					{
						RegionPings->Add(PingTime);
					}
				}
				else
				{
					UE_LOG(LogHathoraSDK, Warning, TEXT("Ping to %s (%s) failed: %s"), *Name, *Host, LexToString(Result.Status));
				}

				// Regardless of whether the ping was successful, we will mark it complete.
				if (++(*CompletedPings) == PingsToComplete)
				{
					UE_LOG(LogHathoraSDK, Log, TEXT("Pings to each region complete; triggering another set of pings."));
					PingEachRegion();
				}
			};

		if (PingType == EHathoraPingType::ICMP)
		{
			FIcmp::IcmpEcho(
				Host,
				GetDefault<UHathoraSDKConfig>()->GetPingTimeoutSeconds(),
				Callback
			);
		}
		else if (PingType == EHathoraPingType::UDPEcho)
		{
			FUDPPing::UDPEcho(
				RegionUrl.Value,
				GetDefault<UHathoraSDKConfig>()->GetPingTimeoutSeconds(),
				Callback
			);
		}
		else
		{
			UE_LOG(LogHathoraSDK, Warning, TEXT("Unsupported ping type: %s"), *UEnum::GetValueAsString(PingType));
		}
	}
}