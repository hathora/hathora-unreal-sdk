// Copyright 2023 Hathora, Inc.

#include "DemoLobbyWidget.h"
#include "Blueprint/WidgetTree.h"
#include "HathoraTypes.h"
#include "GameFramework/HUD.h"
#include "DemoLobbyHUD.h"
#include "HathoraLobbyComponent.h"
#include "HathoraSDK.h"
#include "HathoraSDKAuthV1.h"
#include "../DemoRoomConfigFunctionLibrary.h"
#include "SDemoLobbyLabel.h"
#include "SDemoLobbyButton.h"

void UDemoLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

bool UDemoLobbyWidget::Initialize()
{
	bool Success = Super::Initialize();

	if (!Success) return false;

	// get the string representation of each enum value in EHathoraCloudRegion
	for (uint32 i = 0; i < static_cast<uint8>(EHathoraCloudRegion::Unknown); i++)
	{
		RegionList.Add(MakeShareable(new FString(UHathoraSDK::GetRegionString(static_cast<EHathoraCloudRegion>(i)))));
	}

	if (GetOwningPlayer())
	{
		AHUD* HUD = GetOwningPlayer()->GetHUD();
		if (!HUD) return false;

		ADemoLobbyHUD* DemoLobbyHUD = Cast<ADemoLobbyHUD>(HUD);
		if (!DemoLobbyHUD) return false;

		HathoraLobby = DemoLobbyHUD->GetHathoraLobby();

		HathoraLobby->OnError.AddDynamic(this, &UDemoLobbyWidget::OnError);
		HathoraLobby->OnLobbyReady.AddDynamic(this, &UDemoLobbyWidget::OnLobbyReady);

		StartPings();
	}

	return true;
}

TSharedRef<SWidget> UDemoLobbyWidget::RebuildWidget()
{
	LobbyWidget = SNew(SDemoLobbyWidget)
		.RegionList(&RegionList)
		.OnRefreshClicked_Lambda(
			[this]()
			{
				RefreshPublicLobbies();
				return FReply::Handled();
			}
		)
		.OnLoginClicked_Lambda(
			[this]()
			{
				Login();
				return FReply::Handled();
			}
		)
		.OnSwitchToPublicLobbyClicked_Lambda(
			[this]()
			{
				bCreatePublicLobby = true;
				LobbyWidget->SetLobbyVisibility(true);
				return FReply::Handled();
			}
		)
		.OnSwitchToPrivateLobbyClicked_Lambda(
			[this]()
			{
				bCreatePublicLobby = false;
				LobbyWidget->SetLobbyVisibility(false);
				return FReply::Handled();
			}
		)
		.OnCreateLobbyClicked_Lambda(
			[this]()
			{
				CreateLobby();
				return FReply::Handled();
			}
		)
		.OnJoinClicked_Lambda(
			[this]()
			{
				JoinLobby();
				return FReply::Handled();
			}
		)
		.OnRegionSelected_Lambda(
			[this](TSharedPtr<FString> ProposedSelection, ESelectInfo::Type SelectInfo)
			{
				UpdateSelectedRegionPing();
			}
		);

	return LobbyWidget.ToSharedRef();
}

void UDemoLobbyWidget::OnError(FString InErrorMessage)
{
	if (!LobbyWidget.IsValid())
	{
		return;
	}

	LobbyWidget->SetErrorMessage(InErrorMessage);

	GetWorld()->GetTimerManager().SetTimer(ErrorTimerHandle, [&]()
	{
		if (LobbyWidget.IsValid())
		{
			LobbyWidget->SetErrorMessage(TEXT(""));
		}
	}, 3.0f, false);
}

void UDemoLobbyWidget::OnLobbyReady(
	FHathoraConnectionInfo ConnectionInfo,
	FString RoomConfigString,
	EHathoraCloudRegion Region
) {
	if (!LobbyWidget.IsValid())
	{
		return;
	}

	FDemoRoomConfig RoomConfig = UDemoRoomConfigFunctionLibrary::DeserializeRoomConfigFromString(RoomConfigString);

	// Don't show matches that have started and not accepting any more
	// players (just for demo purposes)
	if (!RoomConfig.bMatchStarted)
	{
		TSharedRef<SDemoLobbyLabel> RoomIdLabel = SNew(SDemoLobbyLabel).LabelText(ConnectionInfo.RoomId);
		TSharedRef<SDemoLobbyLabel> RoomNameLabel = SNew(SDemoLobbyLabel).LabelText(RoomConfig.RoomName);

		int32* RegionPing = Pings.Find(UHathoraSDK::GetRegionString(Region));
		int32 Ping = RegionPing != nullptr ? *RegionPing : 0;
		FString PingString = FString::Printf(TEXT("%dms"), Ping);
		TSharedRef<SDemoLobbyLabel> PingLabel = SNew(SDemoLobbyLabel).LabelText(PingString);

		TSharedRef<SDemoLobbyButton> JoinButton = SNew(SDemoLobbyButton)
			.ButtonText(TEXT("Join"))
			.OnClicked_Lambda(
				[this, ConnectionInfo]()
				{
					HathoraLobby->JoinLobbyWithConnectionInfo(ConnectionInfo);
					return FReply::Handled();
				}
			);

		int32 Row = NumPublicLobbies;
		NumPublicLobbies++;

		LobbyWidget->LobbiesGrid->AddSlot(0, Row)[RoomIdLabel];
		LobbyWidget->LobbiesGrid->AddSlot(1, Row)[RoomNameLabel];
		LobbyWidget->LobbiesGrid->AddSlot(2, Row)[PingLabel];
		LobbyWidget->LobbiesGrid->AddSlot(3, Row)[JoinButton];
	}
}

void UDemoLobbyWidget::StartPings()
{
	UHathoraSDK::GetRegionalPings(
		FHathoraOnGetRegionalPings::CreateLambda(
			[this](const FHathoraRegionPings& Result)
			{
				Pings = Result.Pings;
				UpdateSelectedRegionPing();

				// Ping again in 20 seconds
				GetWorld()->GetTimerManager().SetTimer(PingTimerHandle, [&]()
				{
					StartPings();
				}, 20.0f, false);
			}
		)
	);
}

void UDemoLobbyWidget::UpdateSelectedRegionPing()
{
	if (LobbyWidget.IsValid())
	{
		TSharedPtr<FString> SelectedRegionPtr = LobbyWidget->RegionComboBox->GetSelectedItem();

		if (SelectedRegionPtr.IsValid() && Pings.Contains(*SelectedRegionPtr))
		{
			LobbyWidget->SetRegionPing(Pings[*SelectedRegionPtr]);
		}
		else
		{
			LobbyWidget->SetRegionPing(0);
		}
	}
}

void UDemoLobbyWidget::RefreshPublicLobbies()
{
	if (LobbyWidget.IsValid())
	{
		NumPublicLobbies = 0;
		LobbyWidget->LobbiesGrid->ClearChildren();
		HathoraLobby->FetchPublicLobbies();
	}
}

void UDemoLobbyWidget::Login()
{
	if (!LobbyWidget.IsValid())
	{
		return;
	}

	UHathoraSDK* SDK = HathoraLobby->GetSDK();
	if (!SDK)
	{
		return;
	}

	SDK->AuthV1->LoginAnonymous(
		UHathoraSDKAuthV1::FHathoraOnLogin::CreateLambda(
			[this, SDK](const FHathoraLoginResult& Result)
			{
				if (Result.ErrorMessage.IsEmpty())
				{
					SDK->SetAuthToken(Result.Token);
					LobbyWidget->SetLoggedIn();
				}
				else
				{
					OnError(Result.ErrorMessage);
				}
			}
		)
	);
}

void UDemoLobbyWidget::CreateLobby()
{
	if (LobbyWidget.IsValid())
	{
		FString RoomName = LobbyWidget->RoomNameTextBox->GetText().ToString();
		if (RoomName.IsEmpty())
		{
			OnError(TEXT("Please enter a name for the room."));
		}
		else
		{
			FDemoRoomConfig LobbyConfig;
			LobbyConfig.RoomName = RoomName;
			LobbyConfig.bMatchStarted = false;

			TSharedPtr<FString> SelectedRegion = LobbyWidget->RegionComboBox->GetSelectedItem();
			if (!SelectedRegion.IsValid())
			{
				OnError(TEXT("Please select a region."));
				return;
			}

			EHathoraCloudRegion Region = EHathoraCloudRegion::Unknown;
			for (uint32 i = 0; i < static_cast<uint8>(EHathoraCloudRegion::Unknown); i++)
			{
				FString CurrentRegion = UHathoraSDK::GetRegionString(static_cast<EHathoraCloudRegion>(i));
				if (CurrentRegion == *SelectedRegion)
				{
					Region = static_cast<EHathoraCloudRegion>(i);
					break;
				}
			}

			GetWorld()->GetTimerManager().ClearTimer(ErrorTimerHandle);
			GetWorld()->GetTimerManager().ClearTimer(PingTimerHandle);

			FString SerializedLobbyConfig = UDemoRoomConfigFunctionLibrary::SerializeRoomConfigToString(LobbyConfig, GetWorld());
			HathoraLobby->CreateAndJoinLobby(
				bCreatePublicLobby ? EHathoraLobbyVisibility::Public : EHathoraLobbyVisibility::Private,
				SerializedLobbyConfig,
				Region,
				4
			);

			LobbyWidget->SetCreatingLobby(true);
		}
	}
}

void UDemoLobbyWidget::JoinLobby()
{
	if (LobbyWidget.IsValid())
	{
		FString RoomCode = LobbyWidget->RoomCodeTextBox->GetText().ToString();
		if (RoomCode.IsEmpty())
		{
			OnError(TEXT("Please enter a code for the room."));
		}
		else
		{
			GetWorld()->GetTimerManager().ClearTimer(ErrorTimerHandle);
			GetWorld()->GetTimerManager().ClearTimer(PingTimerHandle);

			HathoraLobby->JoinLobbyWithShortCode(RoomCode);

			LobbyWidget->SetJoiningLobby(true);
		}
	}
}