// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnLive2DAssetModule.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FUnLive2DAssetModule"

DEFINE_LOG_CATEGORY(LogUnLive2D);
DEFINE_LOG_CATEGORY(LogLive2D);

void FUnLive2DAssetModule::StartupModule()
{
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("UnLive2DAsset"));
	if (Plugin.IsValid())
	{
		FString PluginShaderDir = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Shaders"));
		AddShaderSourceDirectoryMapping(TEXT("/Plugin/UnLive2DAsset"), PluginShaderDir);
	}
}

void FUnLive2DAssetModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUnLive2DAssetModule, UnLive2DAsset)