// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DemoMenuWidget.generated.h"

class UVerticalBox;
struct FHathoraRegionPings;

UCLASS()
class UDemoMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void InitiatePing();

protected:
	virtual void NativeConstruct() override;
	virtual bool Initialize() override;

private:
	UPROPERTY()
	UVerticalBox* RegionList;
};
