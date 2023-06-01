#pragma once
#include "CoreMinimal.h"
#include "DiscoveredPingEndpoint.generated.h"

/* TODO - This should really not be a struct that is available for use by consumers of this plugin.
 * Either verify that this is the case (it after all does not use the export macro) or move it.
 */
USTRUCT()
struct FDiscoveredPingEndpoint
{
	GENERATED_BODY()
	
	UPROPERTY()
	FString Region;
	
	UPROPERTY()
	FString Host;
	
	UPROPERTY()
	int32 Port;
};
