// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnLive2DAssetEditor.h"
#include "UnLive2DEditorStyle.h"
#include "UnLive2DTypeAction.h"
#include "UnLive2DEditorSettingsCustomization.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "AssetToolsModule.h"
#include "UnLive2D.h"
#include "UnLive2DMotionTypeAction.h"

#define LOCTEXT_NAMESPACE "FUnLive2DAssetEditorModule"

DEFINE_LOG_CATEGORY(LogUnLive2DEditor);

void FUnLive2DAssetEditorModule::StartupModule()
{
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FUnLive2DAssetEditorModule::OnPostEngineInit);
}

void FUnLive2DAssetEditorModule::ShutdownModule()
{

	FUnLive2DEditorStyle::Shutdown();
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FUnLive2DAssetEditorModule::OnPostEngineInit()
{
	FUnLive2DEditorStyle::Initialize();

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	GameAssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("UnLive2D")), LOCTEXT("UnLive2DCategory", "UnLive2D"));

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	// 为特定类注册自定义资源布局委托
	PropertyModule.RegisterCustomClassLayout("UnLive2D", FOnGetDetailCustomizationInstance::CreateStatic(&FUnLive2DEditorSettingsCustomization::MakeInstance));
	PropertyModule.NotifyCustomizationModuleChanged();

	AssetTools.RegisterAssetTypeActions(MakeShareable(new FUnLive2DTypeAction(GameAssetCategory)));
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FUnLive2DMotionTypeAction(GameAssetCategory)));

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUnLive2DAssetEditorModule, UnLive2DAssetEditor)