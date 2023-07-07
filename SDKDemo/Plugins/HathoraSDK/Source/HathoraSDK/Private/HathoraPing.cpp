// Copyright 2023 Hathora, Inc.
#include "HathoraPing.h"
#include "DiscoveredPingEndpoint.h"
#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "HathoraSDK.h"
#include "PingChecker.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

UHathoraPing::UHathoraPing(const FObjectInitializer& ObjectInitializer)
	: UObject(ObjectInitializer)
{
	this->HathoraSdkConfig = NewObject<UHathoraSdkConfig>();
}

void UHathoraPing::GetRegionalPings(const FOnGetRegionalPingsDelegate& OnComplete)
{

	FHttpRequestRef                                                                Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindLambda([&, OnComplete](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) mutable {
		TArray<FDiscoveredPingEndpoint> PingEndpoints;
		if (bSuccess && Response.IsValid() && FJsonObjectConverter::JsonArrayStringToUStruct(Response->GetContentAsString(), &PingEndpoints))
		{
			PingUrlsAndAggregateTimes(PingEndpoints, this->HathoraSdkConfig->GetPingMeasurementsToTake(), OnComplete);
		}
		else
		{
			UE_LOG(LogHathoraSDK, Warning, TEXT("Could not retrieve ping endpoints"));
			TMap<FString, int32> Pings;
			if (!OnComplete.ExecuteIfBound(Pings))
			{
				UE_LOG(LogHathoraSDK, Warning, TEXT("[GetRegionalPings] function pointer was not valid, so OnComplete will not be called"));
			}
		}
	});

	Request->SetURL(FString::Printf(TEXT("%s/discovery/v1/ping"), *this->HathoraSdkConfig->GetBaseUrl()));
	Request->ProcessRequest();
}

void UHathoraPing::PingUrlsAndAggregateTimes(
	const TArray<FDiscoveredPingEndpoint>& PingEndpoints,
	int32                                  MeasurementsToTake,
	const FOnGetRegionalPingsDelegate&     OnComplete)
{
	TSharedPtr<TMap<FString, int32>> Results = MakeShared<TMap<FString, int32>>();
	TSharedPtr<int32>                CompletedRegions = MakeShared<int32>(0);
	// int32 NumRegionsTopPing = PingEndpoints.Num();

	// aggregate the results of N asynchronous operations into a single TMap.
	for (const FDiscoveredPingEndpoint& PingEndpoint : PingEndpoints)
	{
		FPingChecker* PingChecker = new FPingChecker(PingEndpoint, MeasurementsToTake, FPingChecker::FOnPingCompletionDelegate::CreateLambda( [
			PingEndpoint,
			Results
			](double PingMs, bool bWasSuccessful) {
			if (bWasSuccessful)
			{
				UE_LOG(LogHathoraSDK, Log, TEXT("Median ping to %s (%s:%d) took: %.1f ms"), *PingEndpoint.Region, *PingEndpoint.Host, PingEndpoint.Port, PingMs);
				Results->Add(PingEndpoint.Region, FMath::RoundToInt32(PingMs));
			}
			else
			{
				UE_LOG(LogHathoraSDK, Log, TEXT("Median ping to %s (%s:%d) took: %d ms"), *PingEndpoint.Region, *PingEndpoint.Host, PingEndpoint.Port, PingMs);
			}

		}));
		FRunnableThread* PingCheckThread = FRunnableThread::Create(PingChecker, *FString::Printf(TEXT("PingChecker%s"), *PingEndpoint.Region));
		// TODO - don't block this calling thread
		PingCheckThread->WaitForCompletion();
	}
	(void)OnComplete.ExecuteIfBound(*Results);	
}

