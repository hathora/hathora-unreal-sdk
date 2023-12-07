// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "HathoraSDKAPI.h"
#include "HathoraTypes.h"
#include "Misc/DateTime.h"
#include "HathoraSDKAppV1.generated.h"

USTRUCT(BlueprintType)
struct FHathoraGoogleAuthConfiguration
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString ClientId;
};

USTRUCT(BlueprintType)
struct FHathoraApplicationAuthConfiguration
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	bool bGoogleEnabled = false;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	bool bNicknameEnabled = false;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	bool bAnonymousEnabled = false;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FHathoraGoogleAuthConfiguration Google;
};

USTRUCT(BlueprintType)
struct FHathoraApplication
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	bool bDeleted = false;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString DeletedBy;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FDateTime DeletedAt;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FDateTime CreatedAt;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString CreatedBy;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString OrgId;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FHathoraApplicationAuthConfiguration AuthConfiguration;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString AppSecret;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString AppId;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString AppName;

	// TODO: deployment?
};

USTRUCT(BlueprintType)
struct FHathoraGetAppsResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	int32 StatusCode = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString ErrorMessage;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	TArray<FHathoraApplication> Data;
};

UCLASS(BlueprintType)
class HATHORASDK_API UHathoraSDKAppV1 : public UHathoraSDKAPI
{
	GENERATED_BODY()

public:
	typedef TDelegate<void(const FHathoraGetAppsResult&)> FHathoraOnGetApps;

	// Returns an unsorted list of your organization’s applications.
	// An application is uniquely identified by an appId.
	void GetApps(FHathoraOnGetApps OnComplete);

private:
	static FHathoraApplication ParseApplication(TSharedPtr<FJsonObject> Object);
};
