

using UnrealBuildTool;

public class UnLive2DBlueprintSupport : ModuleRules
{
    public UnLive2DBlueprintSupport(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                "BlueprintGraph",
                "UnLive2DAsset"
            });
    }
}