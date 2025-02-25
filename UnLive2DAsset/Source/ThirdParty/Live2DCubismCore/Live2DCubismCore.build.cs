
using UnrealBuildTool;
using System.IO;

public class Live2DCubismCore : ModuleRules
{
    public Live2DCubismCore(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;

        string Live2DDynDllName = "";
        string Live2DDynamicDllPath = "";

        if (Target.Platform == UnrealTargetPlatform.Win32)
            PublicDefinitions.Add("CSM_CORE_WIN32_DLL=1");
        else
            PublicDefinitions.Add("CSM_CORE_WIN32_DLL=0");

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            Live2DDynDllName = "Live2DCubismCore.dll";
            Live2DDynamicDllPath = Path.Combine(ModuleDirectory, "dll/windows/x86_64", Live2DDynDllName);

            string CubismLibPath = Path.Combine(ModuleDirectory, "lib/windows/x86_64");
            PublicSystemLibraryPaths.Add(CubismLibPath);
            PublicAdditionalLibraries.Add(Path.Combine(CubismLibPath, "142/Live2DCubismCore_MD.lib"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Win32) //UE5不支持Win32 如果你是UE4版本，则可以开放出来
        {
            Live2DDynDllName = "Live2DCubismCore.dll";
            Live2DDynamicDllPath = Path.Combine(ModuleDirectory, "dll/windows/x86", Live2DDynDllName);

            string CubismLibPath = Path.Combine(ModuleDirectory, "lib/windows/x86");
            PublicSystemLibraryPaths.Add(CubismLibPath);
            PublicAdditionalLibraries.Add(Path.Combine(CubismLibPath, "142/Live2DCubismCore_MD.lib"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            Live2DDynDllName = "libLive2DCubismCore.so";
            Live2DDynamicDllPath = Path.Combine(ModuleDirectory, "dll/linux/x86_64", Live2DDynDllName);

            string CubismLibPath = Path.Combine(ModuleDirectory, "lib/linux/x86_64");
            PublicSystemLibraryPaths.Add(CubismLibPath);
            PublicAdditionalLibraries.Add(Path.Combine(CubismLibPath, "libLive2DCubismCore.a"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            Live2DDynDllName = "libLive2DCubismCore.dylib";
            Live2DDynamicDllPath = Path.Combine(ModuleDirectory, "dll/macos", Live2DDynDllName);

            string CubismLibPath = Path.Combine(ModuleDirectory, "lib/macos");
            PublicSystemLibraryPaths.Add(CubismLibPath);
            PublicAdditionalLibraries.Add(Path.Combine(CubismLibPath, "libLive2DCubismCore.a"));
        }
        else if (Target.Platform == UnrealTargetPlatform.IOS)
        {
            if (Target.Configuration == UnrealTargetConfiguration.Debug)
            {
                PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib/ios/Debug-iphoneos", "libLive2DCubismCore.a"));
            }
            else
            {
                PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib/ios/Release-iphoneos", "libLive2DCubismCore.a"));
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {
            string[] Triplets = new string[] {
                "arm64-v8a",
                "armeabi-v7a",
                "x86",
            };

            foreach (var Triplet in Triplets)
            {
                string LibPath = Path.Combine(ModuleDirectory, "lib/android/", Triplet);
                PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libLive2DCubismCore.a"));
            }

            if (Target.Configuration == UnrealTargetConfiguration.Debug)
            {
                foreach (var Triplet in Triplets)
                {
                    string LibPath = Path.Combine(ModuleDirectory, "dll/android/", Triplet);
                    PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libLive2DCubismCore.so"));
                }
            }
        }
        if (Target.Configuration == UnrealTargetConfiguration.Debug)
        {
            PublicDelayLoadDLLs.Add(Live2DDynDllName);
            RuntimeDependencies.Add(Live2DDynamicDllPath);
        }
    }
}
