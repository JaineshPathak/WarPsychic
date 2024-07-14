// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Rouge_like_game : ModuleRules
{
	public Rouge_like_game(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
    }
}
