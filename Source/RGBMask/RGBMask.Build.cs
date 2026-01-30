// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RGBMask : ModuleRules
{
	public RGBMask(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"RGBMask",
			"RGBMask/Variant_Platforming",
			"RGBMask/Variant_Platforming/Animation",
			"RGBMask/Variant_Combat",
			"RGBMask/Variant_Combat/AI",
			"RGBMask/Variant_Combat/Animation",
			"RGBMask/Variant_Combat/Gameplay",
			"RGBMask/Variant_Combat/Interfaces",
			"RGBMask/Variant_Combat/UI",
			"RGBMask/Variant_SideScrolling",
			"RGBMask/Variant_SideScrolling/AI",
			"RGBMask/Variant_SideScrolling/Gameplay",
			"RGBMask/Variant_SideScrolling/Interfaces",
			"RGBMask/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
