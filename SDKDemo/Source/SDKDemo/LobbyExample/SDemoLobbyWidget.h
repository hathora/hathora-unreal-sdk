// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SGridPanel.h"

class SDemoLobbyWidget : public SCompoundWidget
{
public:
	typedef SComboBox<TSharedPtr<FString>> SRegionComboBox;

	SLATE_BEGIN_ARGS(SDemoLobbyWidget)
	{
		_RegionList = new TArray<TSharedPtr<FString>>();
		_OnRefreshClicked = FOnClicked();
		_OnLoginClicked = FOnClicked();
		_OnSwitchToPublicLobbyClicked = FOnClicked();
		_OnSwitchToPrivateLobbyClicked = FOnClicked();
		_OnCreateLobbyClicked = FOnClicked();
		_OnJoinClicked = FOnClicked();
		_OnRegionSelected = SRegionComboBox::FOnSelectionChanged();
	}
	SLATE_ARGUMENT(const TArray<TSharedPtr<FString>>*, RegionList)
	SLATE_EVENT(FOnClicked, OnRefreshClicked)
	SLATE_EVENT(FOnClicked, OnLoginClicked)
	SLATE_EVENT(FOnClicked, OnSwitchToPublicLobbyClicked)
	SLATE_EVENT(FOnClicked, OnSwitchToPrivateLobbyClicked)
	SLATE_EVENT(FOnClicked, OnCreateLobbyClicked)
	SLATE_EVENT(FOnClicked, OnJoinClicked)
	SLATE_EVENT(SRegionComboBox::FOnSelectionChanged, OnRegionSelected)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	TSharedPtr<STextBlock> ErrorMessageBlock;
	TSharedPtr<SWidgetSwitcher> LoginSwitcher;
	TSharedPtr<SRegionComboBox> RegionComboBox;
	TSharedPtr<STextBlock> PublicButtonTextBlock;
	TSharedPtr<STextBlock> PrivateButtonTextBlock;
	TSharedPtr<STextBlock> RegionPingTextBlock;
	TSharedPtr<SEditableTextBox> RoomNameTextBox;
	TSharedPtr<SButton> CreateButton;
	TSharedPtr<STextBlock> CreateButtonTextBlock;
	TSharedPtr<SButton> JoinButton;
	TSharedPtr<STextBlock> JoinButtonTextBlock;
	TSharedPtr<SEditableTextBox> RoomCodeTextBox;
	TSharedPtr<SGridPanel> LobbiesGrid;

	void SetErrorMessage(const FString& Message);
	void SetLoggedIn();
	void SetLobbyVisibility(bool bPublic);
	void SetCreatingLobby(bool bCreating);
	void SetJoiningLobby(bool bJoining);
	void SetRegionPing(int32 Ping);

	TSharedRef<SWidget> HandleGenerateWidget(TSharedPtr<FString> Item) const;
};
