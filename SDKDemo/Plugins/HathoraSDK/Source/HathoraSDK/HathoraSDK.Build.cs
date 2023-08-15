// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HathoraSDK : ModuleRules
{
	public HathoraSDK(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
			}
			);


		PrivateIncludePaths.AddRange(
			new string[] {
			}
			);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"HTTP",
				"Json",
				"JsonUtilities",
				"libWebSockets",
				"SSL",
				"WebSockets",
			}
			);

		AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenSSL", "libWebSockets");
	}
}
