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
#include "Units/UnLive2DRigUnit.h"
#include "RigVMCore/RigVMStruct.h"
#include "AnimGraph/NodeSpawners/UnLive2DAnimUnitNodeSpawner.h"

#define LOCTEXT_NAMESPACE "FUnLive2DAssetEditorModule"

DEFINE_LOG_CATEGORY(LogUnLive2DEditor);

void FUnLive2DAssetEditorModule::StartupModule()
{
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FUnLive2DAssetEditorModule::OnPostEngineInit);
}

void FUnLive2DAssetEditorModule::ShutdownModule()
{
	IKismetCompilerInterface* KismetCompilerModule = FModuleManager::GetModulePtr<IKismetCompilerInterface>("KismetCompiler");
	if (KismetCompilerModule)
	{
		KismetCompilerModule->GetCompilers().Remove(&UnLive2DAnimBlueprintComiler);
	}
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	MessageLogModule.UnregisterLogListing("UnLive2DAnimInstanceLog");

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
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FUnLive2DAnimBlurprintTypeAction(GameAssetCategory)));

	FKismetCompilerContext::RegisterCompilerForBP(UUnLive2DAnimBlueprint::StaticClass(), &FUnLive2DAssetEditorModule::GetUnLive2DAnimCompiler);
	IKismetCompilerInterface& KismetCompilerModule = FModuleManager::LoadModuleChecked<IKismetCompilerInterface>("KismetCompiler");
	KismetCompilerModule.GetCompilers().Add(&UnLive2DAnimBlueprintComiler);
	
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	FMessageLogInitializationOptions InitOptions;
	InitOptions.bShowFilters = true;
	InitOptions.bShowPages = false;
	InitOptions.bAllowClear = true;
	MessageLogModule.RegisterLogListing("UnLive2DAnimInstanceLog", LOCTEXT("UnLive2DAnimInstanceLog", "UUnLive2D Anim Instance Log"), InitOptions);
}

TSharedPtr<FKismetCompilerContext> FUnLive2DAssetEditorModule::GetUnLive2DAnimCompiler(UBlueprint* BP, FCompilerResultsLog& InMessageLog, const FKismetCompilerOptions& InCompileOptions)
{
	return TSharedPtr<FKismetCompilerContext>(new FUnLive2DAnimBlueprintComilerContext(BP, InMessageLog, InCompileOptions));
}

void FUnLive2DAssetEditorModule::GetTypeActions(class UUnLive2DAnimBlueprint* UnLive2DAnimBlue, FBlueprintActionDatabaseRegistrar& ActionRegistrar)
{
	UClass* ActionKey = UnLive2DAnimBlue->GetClass();
	if (!ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		return;
	}

	for (const FRigVMFunction& Function : FRigVMRegistry::Get().GetFunctions())
	{
		UScriptStruct* Struct = Function.Struct;
		if (!Struct->IsChildOf(FUnLive2DRigUnit::StaticStruct()))
		{
			continue;
		}

		FString CategoryMetadata, DisplayNameMetadata, MenuDescSuffixMetadata;
		Struct->GetStringMetaDataHierarchical(FRigVMStruct::CategoryMetaName, &CategoryMetadata);
		Struct->GetStringMetaDataHierarchical(FRigVMStruct::DisplayNameMetaName, &DisplayNameMetadata);
		Struct->GetStringMetaDataHierarchical(FRigVMStruct::MenuDescSuffixMetaName, &MenuDescSuffixMetadata);
		if (!MenuDescSuffixMetadata.IsEmpty())
		{
			MenuDescSuffixMetadata = TEXT(" ") + MenuDescSuffixMetadata;
		}
		FText NodeCategory = FText::FromString(CategoryMetadata);
		FText MenuDesc = FText::FromString(DisplayNameMetadata + MenuDescSuffixMetadata);
		FText ToolTip = Struct->GetToolTipText();

		UUnLive2DAnimUnitNodeSpawner* NodeSpawner = UUnLive2DAnimUnitNodeSpawner::CreateFromStruct(Struct, MenuDesc, NodeCategory, ToolTip);
		check(NodeSpawner != nullptr);
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUnLive2DAssetEditorModule, UnLive2DAssetEditor)