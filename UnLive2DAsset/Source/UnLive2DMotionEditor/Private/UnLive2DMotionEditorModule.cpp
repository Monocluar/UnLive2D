#include "UnLive2DMotionEditorModule.h"
#include "UnLive2DMotionEditorStyle.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "AssetToolsModule.h"
#include "UnLive2DMotionTypeAction.h"

#define LOCTEXT_NAMESPACE "FUnLive2DMotionEditorModule"

void FUnLive2DMotionEditorModule::StartupModule()
{
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FUnLive2DMotionEditorModule::OnPostEngineInit);
}

void FUnLive2DMotionEditorModule::ShutdownModule()
{
}

void FUnLive2DMotionEditorModule::OnPostEngineInit()
{
	FUnLive2DMotionEditorStyle::Initialize();

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	GameAssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("UnLive2D")), LOCTEXT("UnLive2DCategory", "UnLive2D"));

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	AssetTools.RegisterAssetTypeActions(MakeShareable(new FUnLive2DMotionTypeAction(GameAssetCategory)));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUnLive2DMotionEditorModule, UnLive2DMotionEditor)
