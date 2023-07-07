// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "DiscoveredPingEndpoint.h"
#include "HAL/ThreadSafeCounter.h"
#include "HAL/RunnableThread.h"
#include "IWebSocket.h"

/**
 *
 */
class FPingChecker : public FRunnable
{
public:
	typedef TDelegate<void(double /* MedianPingMs */, bool /* bIsSuccess */)> FOnPingCompletionDelegate;
	FPingChecker(const FDiscoveredPingEndpoint& PingEndpoint, const int32 NumMeasurements, const FOnPingCompletionDelegate& OnComplete);
	// ~FPingChecker();

	// virtual bool Init() override;
	virtual uint32 Run() override;
	// virtual void Stop() override;

	// TArray<double> GetPings() const;
private:
	TSharedPtr<FDiscoveredPingEndpoint> PingEndpoint;
	TSharedPtr<IWebSocket>				Socket;
	int32								MeasurementsToTake;
	int32								MeasurementsTaken = 0;
	double								LastPingStartTime = 0;
	// unsure if i need these yet
	// FCriticalSection CriticalSection;
	// FThreadSafeCounter StopTaskCounter;
	TArray<double>			  PingResults;
	FOnPingCompletionDelegate OnComplete;
};
