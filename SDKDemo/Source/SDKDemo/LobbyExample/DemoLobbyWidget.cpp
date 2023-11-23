// Copyright 2023 Hathora, Inc.

#include "DemoLobbyWidget.h"
#include "Blueprint/WidgetTree.h"
#include "SDemoLobbyWidget.h"
#include "HathoraTypes.h"

void UDemoLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

bool UDemoLobbyWidget::Initialize()
{
	bool Success = Super::Initialize();

	if (!Success) return false;

	return true;
}

namespace DemoUtils
{
	static FORCEINLINE FString EnumToString(const FString& EnumName, const uint8 Value)
	{
		const UEnum* EnumPtr = FindObject<UEnum>(GetTransientPackage(), *EnumName, true);
		if (!EnumPtr) return FString("Invalid");

		return EnumPtr->GetNameStringByIndex(Value);
	}
}

TSharedRef<SWidget> UDemoLobbyWidget::RebuildWidget()
{
	// get the string representation of each enum value in EHathoraCloudRegion
	TArray<TSharedPtr<FString>> RegionList;
	for (uint32 i = 0; i < static_cast<uint8>(EHathoraCloudRegion::Unknown); i++)
	{
		RegionList.Add(MakeShareable(new FString(DemoUtils::EnumToString("EHathoraCloudRegion", i))));
	}

	TSharedRef<SDemoLobbyWidget> LobbyWidget = SNew(SDemoLobbyWidget)
		.ErrorMessage(TEXT(""))
		.bIsLoggedIn(false)
		.bCreatePublicLobby(true)
		.RegionList(RegionList);

	return LobbyWidget;
}
