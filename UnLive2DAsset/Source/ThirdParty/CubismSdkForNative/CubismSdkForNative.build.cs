
using UnrealBuildTool;
using System.IO;

public class CubismSdkForNative : ModuleRules
{
    public CubismSdkForNative(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;


        string Live2DDynLibName = "";
        string Live2DDynamicLibPath = "";

        string CubismLibPath = ModuleDirectory + "/Core/";


        PublicIncludePaths.Add(Path.Combine(CubismLibPath, "include"));
#if !UE_5_0_OR_LATER
        if (Target.Platform == UnrealTargetPlatform.Win32) //UE5不支持Win32
        {
            Live2DDynLibName = "Live2DCubismCore.dll";
            Live2DDynamicLibPath = Path.Combine(CubismLibPath, "dll/windows/x86/", Live2DDynLibName);
            if (Target.bBuildEditor)
            {
                PublicAdditionalLibraries.Add(Path.Combine(CubismLibPath, "lib/windows/x86/142/", "Live2DCubismCore_MTd.lib"));
            }
            else
            {
                PublicAdditionalLibraries.Add(Path.Combine(CubismLibPath, "lib/windows/x86/142/", "Live2DCubismCore_MT.lib"));
            }
        }
#else
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {

            PublicDefinitions.Add("CSM_CORE_WIN32_DLL=0");

            Live2DDynLibName = "Live2DCubismCore.dll";
            Live2DDynamicLibPath = Path.Combine(CubismLibPath, "dll/windows/x86_64/", Live2DDynLibName);
            if (Target.bBuildEditor)
            {
                PublicAdditionalLibraries.Add(Path.Combine(CubismLibPath, "lib/windows/x86_64/142/", "Live2DCubismCore_MTd.lib"));
            }
            else
            {
                PublicAdditionalLibraries.Add(Path.Combine(CubismLibPath, "lib/windows/x86_64/142/", "Live2DCubismCore_MT.lib"));
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            Live2DDynLibName = "libLive2DCubismCore.so";
            Live2DDynamicLibPath = Path.Combine(CubismLibPath, "dll/linux/x86_64/", Live2DDynLibName);
            if (Target.bBuildEditor)
            {
                PublicAdditionalLibraries.Add(Path.Combine(CubismLibPath, "lib/linux/x86", "libLive2DCubismCore.a"));
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            Live2DDynLibName = "libLive2DCubismCore.dylib";
            Live2DDynamicLibPath = Path.Combine(CubismLibPath, "dll/macos/", Live2DDynLibName);
            if (Target.bBuildEditor)
            {
                PublicAdditionalLibraries.Add(Path.Combine(CubismLibPath , "lib/macos/", "libLive2DCubismCore.a"));
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.IOS)
        {
            if (Target.bBuildEditor)
            {
                PublicAdditionalLibraries.Add(Path.Combine(CubismLibPath, "lib/ios/Debug-iphoneos", "libLive2DCubismCore.a"));
            }
            else
            {
                PublicAdditionalLibraries.Add(Path.Combine(CubismLibPath, "lib/ios/Release-iphoneos", "libLive2DCubismCore.a"));
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {
            string[] Triplets = new string[] {
                "arm64-v8a",
                "armeabi-v7a",
                "x86",
            };

            if (Target.bBuildEditor)
            {
                foreach (var Triplet in Triplets)
                {
                    string LibPath = Path.Combine(CubismLibPath, "lib/android/", Triplet);
                    PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libLive2DCubismCore.a"));
                }
            }
        }
#endif
        if (Target.bBuildEditor)
        {
            PublicDelayLoadDLLs.Add(Live2DDynLibName);
            RuntimeDependencies.Add(Live2DDynamicLibPath);
        }
    }
}