// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UnLive2DManager : ModuleRules
{
    public UnLive2DManager(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePathModuleNames.AddRange(
            new string[] {
                "AdvancedPreviewScene",
            }
        );

        PublicDependencyModuleNames.AddRange(
            new string[] {
                "AdvancedPreviewScene",
            }
        );

        PrivateIncludePathModuleNames.AddRange(
            new string[] {
                "AssetRegistry", 
                "MainFrame",
                "DesktopPlatform",
                "ContentBrowser",
                "AssetTools",
			}
        );

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "AppFramework",
                "Core",
                "CoreUObject",
                "ApplicationCore",
                "Slate",
                "SlateCore",
                "ContentBrowserData",
                "EditorStyle",
                "Engine",
                "UnrealEd",
                "GraphEditor",
                "InputCore",
                "Kismet",
                "KismetWidgets",
                "AnimGraph",
                "PropertyEditor",
                "EditorWidgets",
                "BlueprintGraph",
                "RHI",
                "UnLive2DAsset"
            }
        );

        DynamicallyLoadedModuleNames.AddRange(
            new string[] {
                "ContentBrowser",
                "Documentation",
                "MainFrame",
                "DesktopPlatform",
                "SkeletonEditor",
                "AssetTools",
			}
        );
    }
}