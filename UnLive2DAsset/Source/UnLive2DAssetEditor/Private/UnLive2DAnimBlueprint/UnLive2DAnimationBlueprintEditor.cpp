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
#include "AnimGraph/UnLive2DAnimGraphNode_Base.h"
#include "Framework/Commands/GenericCommands.h"
#include "UnLive2DAssetEditorModeManager.h"
#include "EdGraphNode_Comment.h"
#include "AnimGraph/UnLive2DAnimStateNode_Base.h"
#include "AnimGraph/UnLive2DAnimStateEntryNode.h"

const FName FUnLive2DAnimBlueprintEditorModes::AnimationBlueprintEditorMode("GraphName");
const FName FUnLive2DAnimBlueprintEditorModes::AnimationBlueprintInterfaceEditorMode("Interface");

#define LOCTEXT_NAMESPACE "UnLive2DAnimationBlueprintEditor"

namespace UnLive2DAnimationBlueprintEditorTabs
{
	const FName DetailsTab(TEXT("DetailsTab"));
	const FName ViewportTab(TEXT("Viewport"));
	const FName AssetBrowserTab(TEXT("SequenceBrowser"));
	const FName CurveNamesTab(TEXT("AnimCurveViewerTab"));
};

FUnLive2DAnimationBlueprintEditor::FUnLive2DAnimationBlueprintEditor()
	: UnLive2DAnimBlueprintEdited(nullptr)
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

void FUnLive2DAnimationBlueprintEditor::SetDetailObjects(const TArray<UObject*>& InObjects)
{
	Inspector->ShowDetailsForObjects(InObjects);
}

void FUnLive2DAnimationBlueprintEditor::SetDetailObject(UObject* Obj)
{
	TArray<UObject*> Objects;
	if (Obj)
	{
		Objects.Add(Obj);
	}
	SetDetailObjects(Objects);
}

void FUnLive2DAnimationBlueprintEditor::HandleObjectsSelected(const TArray<UObject*>& InObjects)
{
	SetDetailObjects(InObjects);
}

void FUnLive2DAnimationBlueprintEditor::HandleObjectSelected(UObject* InObject)
{
	SetDetailObject(InObject);
}

void FUnLive2DAnimationBlueprintEditor::UndoAction()
{
	GEditor->UndoTransaction();
}

void FUnLive2DAnimationBlueprintEditor::RedoAction()
{
	GEditor->RedoTransaction();
}

void FUnLive2DAnimationBlueprintEditor::CreateDefaultCommands()
{
	Super::CreateDefaultCommands();
}

void FUnLive2DAnimationBlueprintEditor::OnCreateGraphEditorCommands(TSharedPtr<FUICommandList> GraphEditorCommandsList)
{

}

void FUnLive2DAnimationBlueprintEditor::CreateDefaultTabContents(const TArray<UBlueprint*>& InBlueprints)
{
	Super::CreateDefaultTabContents(InBlueprints);
}

FGraphAppearanceInfo FUnLive2DAnimationBlueprintEditor::GetGraphAppearance(class UEdGraph* InGraph) const
{
	FGraphAppearanceInfo GraphAppearanceInfo = Super::GetGraphAppearance(InGraph);

	if (GetBlueprintObj()->IsA(UUnLive2DAnimBlueprint::StaticClass()))
	{
		GraphAppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText_UnLive2DAnimation", "ANIMATION");
	}

	return GraphAppearanceInfo;
}

bool FUnLive2DAnimationBlueprintEditor::IsEditable(UEdGraph* InGraph) const
{
	return IsGraphInCurrentBlueprint(InGraph);
}

FText FUnLive2DAnimationBlueprintEditor::GetGraphDecorationString(UEdGraph* InGraph) const
{
	return FText::GetEmpty();
}

void FUnLive2DAnimationBlueprintEditor::OnActiveTabChanged(TSharedPtr<SDockTab> PreviouslyActive, TSharedPtr<SDockTab> NewlyActivated)
{
	if (!NewlyActivated.IsValid())
	{
		TArray<UObject*> ObjArray;
		Inspector->ShowDetailsForObjects(ObjArray);
	}
	else
	{
		FBlueprintEditor::OnActiveTabChanged(PreviouslyActive, NewlyActivated);
	}
}

void FUnLive2DAnimationBlueprintEditor::OnSelectedNodesChangedImpl(const TSet<class UObject*>& NewSelection)
{
	FBlueprintEditor::OnSelectedNodesChangedImpl(NewSelection);

	FUnLive2DAssetEditorModeManager* UnLive2DAssetEditorModeManager = static_cast<FUnLive2DAssetEditorModeManager*>(GetAssetEditorModeManager());

	if (UUnLive2DAnimGraphNode_Base*  SelectedAnimGraphNodePtr = SelectedAnimGraphNode.Get())
	{
		FUnLive2DAnimNode_Base* PreviewNode = FindAnimNode(SelectedAnimGraphNodePtr);
		if (UnLive2DAssetEditorModeManager)
		{
			SelectedAnimGraphNodePtr->OnNodeSelected(false, *UnLive2DAssetEditorModeManager, PreviewNode);
		}
		SelectedAnimGraphNode.Reset();
	}

	// if we only have one node selected, let it know
	UUnLive2DAnimGraphNode_Base* NewSelectedAnimGraphNode = nullptr;
	if (NewSelection.Num() == 1)
	{
		NewSelectedAnimGraphNode = Cast<UUnLive2DAnimGraphNode_Base>(*NewSelection.CreateConstIterator());
		if (NewSelectedAnimGraphNode != nullptr)
		{
			SelectedAnimGraphNode = NewSelectedAnimGraphNode;

			FUnLive2DAnimNode_Base* PreviewNode = FindAnimNode(NewSelectedAnimGraphNode);
			if (PreviewNode && UnLive2DAssetEditorModeManager)
			{
				NewSelectedAnimGraphNode->OnNodeSelected(true, *UnLive2DAssetEditorModeManager, PreviewNode);
			}
		}
	}

	bSelectRegularNode = false;
	for (FGraphPanelSelectionSet::TConstIterator It(NewSelection); It; ++It)
	{
		UEdGraphNode_Comment* SeqNode = Cast<UEdGraphNode_Comment>(*It);
		UUnLive2DAnimStateNode_Base* AnimGraphNodeBase = Cast<UUnLive2DAnimStateNode_Base>(*It);
		UUnLive2DAnimStateEntryNode* AnimStateEntryNode = Cast<UUnLive2DAnimStateEntryNode>(*It);
		if (!SeqNode && !AnimGraphNodeBase && !AnimStateEntryNode)
		{
			bSelectRegularNode = true;
			break;
		}
	}

	if (bHideUnrelatedNodes && !bLockNodeFadeState)
	{
		ResetAllNodesUnrelatedStates();

		if (bSelectRegularNode)
		{
			HideUnrelatedNodes();
		}
	}
}

void FUnLive2DAnimationBlueprintEditor::HandleSetObjectBeingDebugged(UObject* InObject)
{
	Super::HandleSetObjectBeingDebugged(InObject);

	if (UUnLive2DAnimInstance* AnimInstance = Cast<UUnLive2DAnimInstance>(InObject))
	{
		UUnLive2DRendererComponent* RendererComponent = AnimInstance->GetUnLive2DRendererComponent();
		if (RendererComponent)
		{
			// If we are selecting the preview instance, reset us back to 'normal'
			if (InObject->GetWorld()->IsPreviewWorld())
			{

			}
		}
	}
}

FName FUnLive2DAnimationBlueprintEditor::GetToolkitFName() const
{
	return FName("UnLive2DAnimationBlueprintEditor");
}

FText FUnLive2DAnimationBlueprintEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "UnLive2DAnimation Blueprint Editor");
}

FText FUnLive2DAnimationBlueprintEditor::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(GetBlueprintObj());
}

FString FUnLive2DAnimationBlueprintEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "UnLive2DAnimation Blueprint Editor").ToString();
}

FLinearColor FUnLive2DAnimationBlueprintEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor( 0.5f, 0.25f, 0.35f, 0.5f );
}

void FUnLive2DAnimationBlueprintEditor::PostUndo(bool bSuccess)
{
	DocumentManager->CleanInvalidTabs();
	DocumentManager->RefreshAllTabs();

	Super::PostUndo(bSuccess);

	// If we undid a node creation that caused us to clean up a tab/graph we need to refresh the UI state
	RefreshEditors();

	// PostUndo broadcast
	OnPostUndo.Broadcast();

	//RefreshPreviewInstanceTrackCurves();

	//ClearupPreviewMeshAnimNotifyStates();

	OnPostCompile();
}

void FUnLive2DAnimationBlueprintEditor::PostRedo(bool bSuccess)
{
	DocumentManager->RefreshAllTabs();

	Super::PostRedo(bSuccess);

	OnPostUndo.Broadcast();

	//ClearupPreviewMeshAnimNotifyStates();

	OnPostCompile();
}

void FUnLive2DAnimationBlueprintEditor::OnPostCompile()
{
	// act as if we have re-selected, so internal pointers are updated
	if (CurrentUISelection == FBlueprintEditor::SelectionState_Graph)
	{
		FGraphPanelSelectionSet SelectionSet = GetSelectedNodes();
		OnSelectedNodesChangedImpl(SelectionSet);
		FocusInspectorOnGraphSelection(SelectionSet, /*bForceRefresh=*/ true);
	}

	// if the user manipulated Pin values directly from the node, then should copy updated values to the internal node to retain data consistency
	UEdGraph* FocusedGraph = GetFocusedGraph();
	if (FocusedGraph)
	{
		// find UAnimGraphNode_Base
		for (UEdGraphNode* Node : FocusedGraph->Nodes)
		{
			UUnLive2DAnimGraphNode_Base* AnimGraphNode = Cast<UUnLive2DAnimGraphNode_Base>(Node);
			if (AnimGraphNode)
			{
				FUnLive2DAnimNode_Base* AnimNode = FindAnimNode(AnimGraphNode);
				if (AnimNode)
				{
					AnimGraphNode->CopyNodeDataToPreviewNode(AnimNode);
				}
			}
		}
	}
}

FUnLive2DAnimNode_Base* FUnLive2DAnimationBlueprintEditor::FindAnimNode(class UUnLive2DAnimGraphNode_Base* AnimGraphNode) const
{
	FUnLive2DAnimNode_Base* AnimNode = nullptr;
	if (AnimGraphNode)
	{

	}

	return AnimNode;
}

void FUnLive2DAnimationBlueprintEditor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

TStatId FUnLive2DAnimationBlueprintEditor::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FUnLive2DAnimationBlueprintEditor, STATGROUP_Tickables);
}

void FUnLive2DAnimationBlueprintEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_UnLive2DAnimationBlueprintEditor", "UnLive2DAnimation Blueprint Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	Super::RegisterTabSpawners(InTabManager);

}

void FUnLive2DAnimationBlueprintEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	Super::UnregisterTabSpawners(InTabManager);
}

UBlueprint* FUnLive2DAnimationBlueprintEditor::GetBlueprintObj() const
{
	const TArray<UObject*>& EditingObjs = GetEditingObjects();
	for (int32 i = 0; i < EditingObjs.Num(); ++i)
	{
		if (EditingObjs[i]->IsA<UUnLive2DAnimBlueprint>()) { return (UBlueprint*)EditingObjs[i]; }
	}
	return nullptr;
}

void FUnLive2DAnimationBlueprintEditor::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged)
{
	Super::NotifyPostChange(PropertyChangedEvent, PropertyThatChanged);

	// When you change properties on a node, call CopyNodeDataToPreviewNode to allow pushing those to preview instance, for live editing
	UUnLive2DAnimGraphNode_Base* SelectedNode = SelectedAnimGraphNode.Get();
	if (SelectedNode)
	{
		FUnLive2DAnimNode_Base* PreviewNode = FindAnimNode(SelectedNode);
		if (PreviewNode)
		{
			SelectedNode->CopyNodeDataToPreviewNode(PreviewNode);
		}
	}
}

void FUnLive2DAnimationBlueprintEditor::BindCommands()
{

}

void FUnLive2DAnimationBlueprintEditor::HandleViewportCreated(const TSharedRef<class SCompoundWidget>& InPersonaViewport)
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

	ToolbarExtender->AddToolBarExtension
	(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateLambda([this](FToolBarBuilder& ParentToolbarBuilder)
	{
		FUnLive2DManagerModule& MangerModule = FModuleManager::LoadModuleChecked<FUnLive2DManagerModule>("UnLive2DManager");
		TSharedRef<IUnLive2DAssetFamily> AssetFamily = MangerModule.CreatePersonaAssetFamily(UnLive2DAnimBlueprintEdited);
		AddToolbarWidget(MangerModule.CreateAssetFamilyShortcutWidget(SharedThis(this), AssetFamily));
	}));
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
	UnLive2DAnimBlueprintEdited = InAnimBlueprint;
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

	/*if (InAnimBlueprint->BlueprintType == BPTYPE_Interface)
	{
		AddApplicationMode(FUnLive2DAnimBlueprintEditorModes::AnimationBlueprintInterfaceEditorMode, MakeShareable(new FUnLive2DAnimBlueprintInterfaceEditorMode(SharedThis(this))));

		ExtendMenu();
		ExtendToolbar();
		RegenerateMenusAndToolbars();

		// Activate the initial mode (which will populate with a real layout)
		SetCurrentMode(FUnLive2DAnimBlueprintEditorModes::AnimationBlueprintInterfaceEditorMode);
	}*/
	AddApplicationMode(FUnLive2DAnimBlueprintEditorModes::AnimationBlueprintEditorMode, MakeShareable(new FUnLive2DAnimBlueprintEditorMode(SharedThis(this))));
	UUnLive2DAnimBlueprint* AnimBlueprint = UnLive2DManagerToolkit->GetAnimBlueprint();
	UUnLive2DAnimBlueprint*  PreviewAnimBlueprint = AnimBlueprint->GetPreviewAnimationBlueprint();

	ExtendMenu();
	ExtendToolbar();
	RegenerateMenusAndToolbars();

	// Activate the initial mode (which will populate with a real layout)
	SetCurrentMode(FUnLive2DAnimBlueprintEditorModes::AnimationBlueprintEditorMode);

	// Post-layout initialization
	PostLayoutBlueprintEditorInitialization();

	MangerModule.CustomizeBlueprintEditorDetails(Inspector->GetPropertyView().ToSharedRef(), FOnInvokeTab::CreateSP(this, &FAssetEditorToolkit::InvokeTab));
	/*

	if (bNewlyCreated && InAnimBlueprint->BlueprintType == BPTYPE_Interface)
	{
		NewDocument_OnClick(CGT_NewAnimationLayer);
	}*/
}

#undef LOCTEXT_NAMESPACE