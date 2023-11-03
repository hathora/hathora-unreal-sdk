// Copyright 2023 Hathora, Inc.

#pragma once

#include "CoreMinimal.h"
#include "HathoraSDKAPI.h"
#include "HathoraTypes.h"
#include "HathoraSDKAuthV1.generated.h"

USTRUCT(BlueprintType)
struct FHathoraLoginResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	int32 StatusCode = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString ErrorMessage;

	// A unique Hathora-signed JWT player token.
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FString Token;
};

UCLASS(BlueprintType)
class HATHORASDK_API UHathoraSDKAuthV1 : public UHathoraSDKAPI
{
	GENERATED_BODY()

public:
	UDELEGATE()
	DECLARE_DYNAMIC_DELEGATE_OneParam(FHathoraOnLogin, FHathoraLoginResult, Result);

	// Returns a unique player token for an anonymous user.
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | AuthV1")
	void LoginAnonymous(FHathoraOnLogin OnComplete);

	// Returns a unique player token with a specified nickname for a user.
	// @param Nickname An alias to represent a player.
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | AuthV1")
	void LoginNickname(FString Nickname, FHathoraOnLogin OnComplete);

	// Returns a unique player token using a Google-signed OIDC idToken.
	// @param IdToken A Google-signed OIDC ID token representing a player's
	//                authenticated identity. Learn how to get an idToken
	//                [here](https://cloud.google.com/docs/authentication/get-id-token).
	UFUNCTION(BlueprintCallable, Category = "HathoraSDK | AuthV1")
	void LoginGoogle(FString IdToken, FHathoraOnLogin OnComplete);

private:
	void Login(FString Path, FJsonObject Body, FHathoraOnLogin OnComplete);
};
