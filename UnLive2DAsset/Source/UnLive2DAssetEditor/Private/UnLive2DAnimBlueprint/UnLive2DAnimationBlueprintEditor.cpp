#include "UnLive2DAnimationBlueprintEditor.h"
#include "Animation/UnLive2DAnimBlueprint.h"
#include "SBlueprintEditorToolbar.h"
#include "UnLive2DManagerModule.h"
#include "UnLive2DAnimBlueprintInterfaceEditorMode.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Animation/UnLive2DAnimInstance.h"
#include "EditorReimportHandler.h"
#include "IUnLive2DToolkit.h"
#include "BlueprintEditor.h"
#include "SKismetInspector.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "IUnLive2DAssetFamily.h"
#include "UnLive2DAnimBlueprintEditorMode.h"
#include "EdGraphSchema_K2.h"

const FName FUnLive2DAnimBlueprintEditorModes::AnimationBlueprintEditorMode("GraphName");
const FName FUnLive2DAnimBlueprintEditorModes::AnimationBlueprintInterfaceEditorMode("Interface");

namespace UnLive2DAnimationBlueprintEditorTabs
{
	const FName DetailsTab(TEXT("DetailsTab"));
	const FName SkeletonTreeTab(TEXT("SkeletonTreeView"));
	const FName ViewportTab(TEXT("Viewport"));
	const FName AdvancedPreviewTab(TEXT("AdvancedPreviewTab"));
	const FName AssetBrowserTab(TEXT("SequenceBrowser"));
	const FName AnimBlueprintPreviewEditorTab(TEXT("AnimBlueprintPreviewEditor"));
	const FName AssetOverridesTab(TEXT("AnimBlueprintParentPlayerEditor"));
	const FName SlotNamesTab(TEXT("SkeletonSlotNames"));
	const FName CurveNamesTab(TEXT("AnimCurveViewerTab"));
};

FUnLive2DAnimationBlueprintEditor::FUnLive2DAnimationBlueprintEditor()
{
	GEditor->OnBlueprintPreCompile().AddRaw(this, &FUnLive2DAnimationBlueprintEditor::OnBlueprintPreCompile);
	LastGraphPinType.ResetToDefaults();
	LastGraphPinType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
}

FUnLive2DAnimationBlueprintEditor::~FUnLive2DAnimationBlueprintEditor()
{
	GEditor->OnBlueprintPreCompile().RemoveAll(this);

	GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetPostImport.RemoveAll(this);
	FReimportManager::Instance()->OnPostReimport().RemoveAll(this);
}

void FUnLive2DAnimationBlueprintEditor::BindCommands()
{

}

void FUnLive2DAnimationBlueprintEditor::ExtendMenu()
{
	if (MenuExtender.IsValid())
	{
		RemoveMenuExtender(MenuExtender);
		MenuExtender.Reset();
	}

	MenuExtender = MakeShareable(new FExtender);
	AddMenuExtender(MenuExtender);

	/*// add extensible menu if exists
	FAnimationBlueprintEditorModule& AnimationBlueprintEditorModule = FModuleManager::LoadModuleChecked<FAnimationBlueprintEditorModule>("AnimationBlueprintEditor");
	AddMenuExtender(AnimationBlueprintEditorModule.GetMenuExtensibilityManager()->GetAllExtenders(GetToolkitCommands(), GetEditingObjects()));*/
}

void FUnLive2DAnimationBlueprintEditor::ExtendToolbar()
{
	// If the ToolbarExtender is valid, remove it before rebuilding it
	if (ToolbarExtender.IsValid())
	{
		RemoveToolbarExtender(ToolbarExtender);
		ToolbarExtender.Reset();
	}

	ToolbarExtender = MakeShareable(new FExtender);

	AddToolbarExtender(ToolbarExtender);

	/*FAnimationBlueprintEditorModule& AnimationBlueprintEditorModule = FModuleManager::LoadModuleChecked<FAnimationBlueprintEditorModule>("AnimationBlueprintEditor");
	AddToolbarExtender(AnimationBlueprintEditorModule.GetToolBarExtensibilityManager()->GetAllExtenders(GetToolkitCommands(), GetEditingObjects()));*/

	UUnLive2DAnimBlueprint* AnimBlueprint = UnLive2DManagerToolkit->GetAnimBlueprint();
	if (AnimBlueprint && AnimBlueprint->BlueprintType != BPTYPE_Interface)
	{
		ToolbarExtender->AddToolBarExtension(
			"Asset",
			EExtensionHook::After,
			GetToolkitCommands(),
			FToolBarExtensionDelegate::CreateLambda([this](FToolBarBuilder& ParentToolbarBuilder)
		{
			FUnLive2DManagerModule& MangerModule = FModuleManager::LoadModuleChecked<FUnLive2DManagerModule>("UnLive2DManager");
			/*FPersonaModule::FCommonToolbarExtensionArgs Args;
			Args.bPreviewAnimation = false;
			PersonaModule.AddCommonToolbarExtensions(ParentToolbarBuilder, PersonaToolkit.ToSharedRef(), Args);*/

			TSharedRef<class IUnLive2DAssetFamily> AssetFamily = MangerModule.CreatePersonaAssetFamily(GetBlueprintObj());
			AddToolbarWidget(MangerModule.CreateAssetFamilyShortcutWidget(SharedThis(this), AssetFamily));
		}
		));
	}
}

void FUnLive2DAnimationBlueprintEditor::OnBlueprintPreCompile(UBlueprint* BlueprintToCompile)
{
	if (GetObjectsCurrentlyBeingEdited()->Num() > 0 && BlueprintToCompile == GetBlueprintObj())
	{
		// Grab the currently debugged object, so we can re-set it below in OnBlueprintPostCompile
		DebuggedUnLive2DComponent = nullptr;

		UUnLive2DAnimInstance* CurrentDebugObject = Cast<UUnLive2DAnimInstance>(BlueprintToCompile->GetObjectBeingDebugged());
		if (CurrentDebugObject)
		{
			// Force close any asset editors that are using the AnimScriptInstance (such as the Property Matrix), the class will be garbage collected
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseOtherEditors(CurrentDebugObject, nullptr);
			DebuggedUnLive2DComponent = CurrentDebugObject->GetUnLive2DRendererComponent();
		}
	}
}

void FUnLive2DAnimationBlueprintEditor::InitUnLive2DAnimationBlueprintEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, class UUnLive2DAnimBlueprint* InAnimBlueprint)
{
	// 记录我们是否是新创建的
	bool bNewlyCreated = InAnimBlueprint->bIsNewlyCreated;
	InAnimBlueprint->bIsNewlyCreated = false;

	if (!Toolbar.IsValid())
	{
		Toolbar = MakeShareable(new FBlueprintEditorToolbar(SharedThis(this)));
	}

	FUnLive2DManagerModule& MangerModule = FModuleManager::LoadModuleChecked<FUnLive2DManagerModule>("UnLive2DManager");
	UnLive2DManagerToolkit = MangerModule.CreatePersonaToolkit(InAnimBlueprint);

	TSharedRef<IUnLive2DAssetFamily> AssetFamily = MangerModule.CreatePersonaAssetFamily(InAnimBlueprint);
	AssetFamily->RecordAssetOpened(FAssetData(InAnimBlueprint));

	if (InAnimBlueprint->BlueprintType != BPTYPE_Interface)
	{
	}

	// Build up a list of objects being edited in this asset editor
	TArray<UObject*> ObjectsBeingEdited;
	ObjectsBeingEdited.Add(InAnimBlueprint);

	CreateDefaultCommands();

	BindCommands();

	RegisterMenus();

	const FName UnLive2DAnimationBlueprintEditorAppName(TEXT("UnLive2DAnimationBlueprintEditorApp"));

	// Initialize the asset editor and spawn tabs
	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;
	InitAssetEditor(Mode, InitToolkitHost, UnLive2DAnimationBlueprintEditorAppName, FTabManager::FLayout::NullLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, ObjectsBeingEdited);

	TArray<UBlueprint*> AnimBlueprints;
	AnimBlueprints.Add(InAnimBlueprint);

	CommonInitialization(AnimBlueprints, /*bShouldOpenInDefaultsMode=*/ false);

	if (InAnimBlueprint->BlueprintType == BPTYPE_Interface)
	{
		AddApplicationMode(FUnLive2DAnimBlueprintEditorModes::AnimationBlueprintInterfaceEditorMode, MakeShareable(new FUnLive2DAnimBlueprintInterfaceEditorMode(SharedThis(this))));

		ExtendMenu();
		ExtendToolbar();
		RegenerateMenusAndToolbars();

		// Activate the initial mode (which will populate with a real layout)
		SetCurrentMode(FUnLive2DAnimBlueprintEditorModes::AnimationBlueprintInterfaceEditorMode);
	}
	else
	{
		AddApplicationMode(FUnLive2DAnimBlueprintEditorModes::AnimationBlueprintEditorMode, MakeShareable(new FUnLive2DAnimBlueprintEditorMode(SharedThis(this))));
		UUnLive2DAnimBlueprint* AnimBlueprint = UnLive2DManagerToolkit->GetAnimBlueprint();
		UUnLive2DAnimBlueprint*  PreviewAnimBlueprint = AnimBlueprint->GetPreviewAnimationBlueprint();

		ExtendMenu();
		ExtendToolbar();
		RegenerateMenusAndToolbars();

		// Activate the initial mode (which will populate with a real layout)
		SetCurrentMode(FUnLive2DAnimBlueprintEditorModes::AnimationBlueprintEditorMode);
	}

	// Post-layout initialization
	PostLayoutBlueprintEditorInitialization();

	MangerModule.CustomizeBlueprintEditorDetails(Inspector->GetPropertyView().ToSharedRef(), FOnInvokeTab::CreateSP(this, &FAssetEditorToolkit::InvokeTab));

	if (bNewlyCreated && InAnimBlueprint->BlueprintType == BPTYPE_Interface)
	{
		NewDocument_OnClick(CGT_NewAnimationLayer);
	}
}
