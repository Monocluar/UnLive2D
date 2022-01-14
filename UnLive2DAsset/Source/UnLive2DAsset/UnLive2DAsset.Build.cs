// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System;
using System.IO;

public class UnLive2DAsset : ModuleRules
{
	public UnLive2DAsset(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        bUseUnity = false;

        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private/Framework/src"));

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",

				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "RHI",
                "RenderCore",
                "Projects",

                "CubismSdkForNative"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
    }
}
