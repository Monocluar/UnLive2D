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
                "ProceduralMeshComponent",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
            {
                "CoreUObject",
                "Engine",
                "UMG",
                "Slate",
                "SlateCore",
                "RHI",
                "RenderCore",
                "Projects",
				"RigVM",
                "CubismSdkForNative"
				// ... add private dependencies that you statically link with here ...	
			}
			);


        if (Target.bBuildEditor == true)
        {
            PrivateDependencyModuleNames.AddRange(
            new string[]{
                "UnrealEd",
                "RigVMDeveloper",
				}
            );

            PrivateIncludePathModuleNames.Add("UnLive2DAssetEditor");
            DynamicallyLoadedModuleNames.Add("UnLive2DAssetEditor");
        }
    }
}