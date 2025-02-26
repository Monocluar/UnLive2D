// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnLive2DAssetEditor.h"
#include "UnLive2DEditorStyle.h"
#include "UnLive2DTypeAction.h"
#include "UnLive2DEditorSettingsCustomization.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "AssetToolsModule.h"
#include "UnLive2D.h"
#include "UnLive2DMotionTypeAction.h"
#include "Animation/UnLive2DAnimBlueprint.h"
#include "UnLive2DAnimBlueprint/UnLive2DAnimBlurprintTypeAction.h"
#include "KismetCompiler.h"
#include "KismetCompilerModule.h"
#include "MessageLogModule.h"
#include "MessageLogInitializationOptions.h"
#include "RigVMCore/RigVMFunction.h"
#include "RigVMCore/RigVMRegistry.h"
#include "RigVMCore/RigVMStruct.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintGraphNodeFactory.h"
#include "UnLive2DExpressionAction.h"
#include "ISettingsModule.h"
#include "UnLive2DSetting.h"
#include "ThumbnailRenderer/UnLive2DThumbnailRenderer.h"

#define LOCTEXT_NAMESPACE "FUnLive2DAssetEditorModule"

DEFINE_LOG_CATEGORY(LogUnLive2DEditor);

void FUnLive2DAssetEditorModule::StartupModule()
{
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FUnLive2DAssetEditorModule::OnPostEngineInit);
}

void FUnLive2DAssetEditorModule::ShutdownModule()
{

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "UnLive2DAsset");
	}
	FUnLive2DEditorStyle::Shutdown();
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FUnLive2DAssetEditorModule::OnPostEngineInit()
{
	FUnLive2DEditorStyle::Initialize();

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	GameAssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("UnLive2D")), LOCTEXT("UnLive2DCategory", "UnLive2D"));

	TSharedPtr<FUnLive2DAnimBlueprintGraphNodeFactory> UnLive2DAnimBlueprintGraphNodeFactory = MakeShared<FUnLive2DAnimBlueprintGraphNodeFactory>();
	FEdGraphUtilities::RegisterVisualNodeFactory(UnLive2DAnimBlueprintGraphNodeFactory);

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	// 为特定类注册自定义资源布局委托
	PropertyModule.RegisterCustomClassLayout("UnLive2D", FOnGetDetailCustomizationInstance::CreateStatic(&FUnLive2DEditorSettingsCustomization::MakeInstance));
	PropertyModule.NotifyCustomizationModuleChanged();

	AssetTools.RegisterAssetTypeActions(MakeShareable(new FUnLive2DTypeAction(GameAssetCategory)));
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FUnLive2DMotionTypeAction(GameAssetCategory)));
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FUnLive2DExpressionAction(GameAssetCategory)));
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FUnLive2DAnimBlurprintTypeAction(GameAssetCategory)));

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "UnLive2DAsset", LOCTEXT("RuntimeSettingsName", "UnLive2D Settings"),
			LOCTEXT("RuntimeSettingsDescription", "Configure the UnLive2DAsset plugin"), GetMutableDefault<UUnLive2DSetting>());
	}

	//UThumbnailManager::Get().RegisterCustomRenderer(UUnLive2D::StaticClass(), UUnLive2DThumbnailRenderer::StaticClass());
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUnLive2DAssetEditorModule, UnLive2DAssetEditor)