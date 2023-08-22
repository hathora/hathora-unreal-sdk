// Copyright 2023 Hathora, Inc. All Rights Reserved.

#pragma once

#include "Ssl.h"

// Work around a conflict between a UI namespace defined by engine code and a typedef in OpenSSL
#define UI UI_ST
// Work around assertion macros in ue4
#undef verify
THIRD_PARTY_INCLUDES_START
#include "libwebsockets.h"
THIRD_PARTY_INCLUDES_END
#undef UI

struct CollectPingState {
	bool PingSent;
	bool Disconnected;
	double StartTime;
	int32 PingTimeMs;
};

lws* CollectRegionPingConnect(lws_context* context, const std::string& host, int port, CollectPingState* state);
int CollectRegionPingsCallback(lws* Connection, lws_callback_reasons Reason, void* UserData, void* Data, size_t Length);
int CollectRegionPing(const FDiscoveredPingEndpoint& Endpoint);
