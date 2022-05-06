// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UnLive2DAssetEditor : ModuleRules
{
	public UnLive2DAssetEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

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
				"UnLive2DAsset",
				"PropertyEditor",
                "UnrealEd",
                "AssetTools",
                "EditorWidgets",
                "EditorStyle",
                "InputCore",
				"Projects",
				"KismetWidgets",
				"RenderCore",
				"MainFrame",
				"Documentation",
				"ApplicationCore",
				"Kismet",
				"BlueprintGraph",
				"ContentBrowser",
				"ToolMenus",
				"MessageLog",
				"KismetCompiler",
				"RigVM",
				"RigVMDeveloper",
				"GraphEditor",
				"UnLive2DManager",
				// ... add private dependencies that you statically link with here ...	
			}
			);

    }
}
