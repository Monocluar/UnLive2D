#include "UnLive2DAnimationBlueprintEditor.h"
#include "Animation/UnLive2DAnimBlueprint.h"
#include "SBlueprintEditorToolbar.h"
#include "UnLive2DManagerModule.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "EditorReimportHandler.h"
#include "IUnLive2DToolkit.h"
#include "BlueprintEditor.h"
#include "SKismetInspector.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "IUnLive2DAssetFamily.h"
#include "Framework/Commands/GenericCommands.h"
#include "UnLive2DAssetEditorModeManager.h"
#include "SUnLive2DAnimBlueprintEditorViewport.h"
#include "Animation/UnLive2DMotion.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "UnLive2DEditorStyle.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintGraphNode_Root.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintGraphNode.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintNode_Base.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintNode_MotionPlayer.h"
#include "EdGraphUtilities.h"
#include "HAL/PlatformApplicationMisc.h"
#include "SNodePanel.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Misc/EngineVersionComparison.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintEditorCommands.h"
#include "GraphEditorActions.h"

const FName FUnLive2DAnimBlueprintEditorModes::AnimationBlueprintEditorMode("GraphName");
const FName FUnLive2DAnimBlueprintEditorModes::AnimationBlueprintInterfaceEditorMode("Interface");

#define LOCTEXT_NAMESPACE "UnLive2DAnimationBlueprintEditor"

namespace UnLive2DAnimationBlueprintEditorTabs
{
	const FName ViewportTab(TEXT("Viewport"));
	const FName AssetBrowserTab(TEXT("SequenceBrowser"));
	const FName CurveNamesTab(TEXT("AnimCurveViewerTab"));
	const FName GraphDocumentTab(TEXT("GraphDocumentTab"));
	const FName PropertiesTab(TEXT("PropertiesTab"));
};

FUnLive2DAnimationBlueprintEditor::FUnLive2DAnimationBlueprintEditor()
	: UnLive2DAnimBlueprintEdited(nullptr)
{
}

FUnLive2DAnimationBlueprintEditor::~FUnLive2DAnimationBlueprintEditor()
{
	GEditor->OnBlueprintPreCompile().RemoveAll(this);

	GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetPostImport.RemoveAll(this);
	FReimportManager::Instance()->OnPostReimport().RemoveAll(this);
}

void FUnLive2DAnimationBlueprintEditor::UndoAction()
{
	GEditor->UndoTransaction();
}

void FUnLive2DAnimationBlueprintEditor::RedoAction()
{
	GEditor->RedoTransaction();
}

void FUnLive2DAnimationBlueprintEditor::SetSelection(TArray<UObject*> SelectedObjects)
{
	if (UnLive2DAnimProperties.IsValid())
	{
		UnLive2DAnimProperties->SetObjects(SelectedObjects);
	}
}

int32 FUnLive2DAnimationBlueprintEditor::GetNumberOfSelectedNodes() const
{
	if (UnLive2DAnimBlueprintGraphEditor.IsValid())
	{
		return UnLive2DAnimBlueprintGraphEditor->GetSelectedNodes().Num();
	}
	return 0;
}

FGraphPanelSelectionSet FUnLive2DAnimationBlueprintEditor::GetSelectedNodes() const
{
	FGraphPanelSelectionSet CurrentSelection;
	if (UnLive2DAnimBlueprintGraphEditor.IsValid())
	{
		CurrentSelection = UnLive2DAnimBlueprintGraphEditor->GetSelectedNodes();
	}
	return CurrentSelection;
}

bool FUnLive2DAnimationBlueprintEditor::CanPasteNodes() const
{
	FString ClipboardContent;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);

	return FEdGraphUtilities::CanImportNodesFromText(UnLive2DAnimBlueprintEdited->GetGraph(), ClipboardContent);
}

void FUnLive2DAnimationBlueprintEditor::PasteNodesHere(const FVector2D& Location)
{
	const FScopedTransaction Transaction(LOCTEXT("UnLive2DAnimationBlueprintEditorPaste", "Paste Animation Blueprint Node"));
	UnLive2DAnimBlueprintEdited->GetGraph()->Modify();
	UnLive2DAnimBlueprintEdited->Modify();

	// 清除选择集
	UnLive2DAnimBlueprintGraphEditor->ClearSelectionSet();

	// 抓取要粘贴的文本
	FString TextToImport;
	FPlatformApplicationMisc::ClipboardPaste(TextToImport);

	// 导入节点
	TSet<UEdGraphNode*> PastedNodes;
	FEdGraphUtilities::ImportNodesFromText(UnLive2DAnimBlueprintEdited->GetGraph(), TextToImport, /*out*/ PastedNodes);

	// 节点的平均位置，以便在保持彼此相对距离的同时移动节点
	FVector2D AvgNodePosition(0.0f, 0.0f);

	for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
	{
		UEdGraphNode* Node = *It;
		AvgNodePosition.X += Node->NodePosX;
		AvgNodePosition.Y += Node->NodePosY;
	}

	if (PastedNodes.Num() > 0)
	{
		float InvNumNodes = 1.0f / float(PastedNodes.Num());
		AvgNodePosition.X *= InvNumNodes;
		AvgNodePosition.Y *= InvNumNodes;
	}


	for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
	{
		UEdGraphNode* Node = *It;

		if (UUnLive2DAnimBlueprintGraphNode* AnimBlueprintGraphNode = Cast<UUnLive2DAnimBlueprintGraphNode>(Node))
		{
			UnLive2DAnimBlueprintEdited->GetGraphAllNodes().Add(AnimBlueprintGraphNode->AnimBlueprintNode);
		}

		// Select the newly pasted stuff
		UnLive2DAnimBlueprintGraphEditor->SetNodeSelection(Node, true);

		Node->NodePosX = (Node->NodePosX - AvgNodePosition.X) + Location.X;
		Node->NodePosY = (Node->NodePosY - AvgNodePosition.Y) + Location.Y;

		Node->SnapToGrid(SNodePanel::GetSnapGridSize());

		// Give new node a different Guid from the old one
		Node->CreateNewGuid();
	}
	// 刷新节点
	UnLive2DAnimBlueprintEdited->CompileUnLive2DAnimNodesFromGraphNodes();

	// 更新图表UI
	UnLive2DAnimBlueprintGraphEditor->NotifyGraphChanged();

	UnLive2DAnimBlueprintEdited->PostEditChange();
	UnLive2DAnimBlueprintEdited->MarkPackageDirty();
}

bool FUnLive2DAnimationBlueprintEditor::GetBoundsForSelectedNodes(class FSlateRect& Rect, float Padding)
{
	return UnLive2DAnimBlueprintGraphEditor->GetBoundsForSelectedNodes( Rect, Padding);
}

FName FUnLive2DAnimationBlueprintEditor::GetToolkitFName() const
{
	return FName("UnLive2DAnimationBlueprintEditor");
}

FText FUnLive2DAnimationBlueprintEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "UnLive2DAnimation Blueprint Editor");
}

FText FUnLive2DAnimationBlueprintEditor::GetToolkitName() const
{
	return FText::FromString(UnLive2DAnimBlueprintEdited->GetName());
}

FText FUnLive2DAnimationBlueprintEditor::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(UnLive2DAnimBlueprintEdited);
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
	// PostUndo broadcast
	OnPostUndo.Broadcast();

	//RefreshPreviewInstanceTrackCurves();

	//ClearupPreviewMeshAnimNotifyStates();

	OnPostCompile();
}

void FUnLive2DAnimationBlueprintEditor::PostRedo(bool bSuccess)
{

	OnPostUndo.Broadcast();

	//ClearupPreviewMeshAnimNotifyStates();

	OnPostCompile();
}

void FUnLive2DAnimationBlueprintEditor::OnPostCompile()
{
	
}


void FUnLive2DAnimationBlueprintEditor::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged)
{
	if (UnLive2DAnimBlueprintGraphEditor.IsValid() && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		UnLive2DAnimBlueprintGraphEditor->NotifyGraphChanged();
	}
}

void FUnLive2DAnimationBlueprintEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_UnLive2DAnimationBlueprintEditor", "UnLive2DAnimation Blueprint Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	DocumentManager->SetTabManager(InTabManager);

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(UnLive2DAnimationBlueprintEditorTabs::ViewportTab, FOnSpawnTab::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("ViewportTabTitle", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));

	InTabManager->RegisterTabSpawner(UnLive2DAnimationBlueprintEditorTabs::AssetBrowserTab, FOnSpawnTab::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::SpawnTab_AssetBrowser))
		.SetDisplayName(NSLOCTEXT("PersonaModes", "AssetBrowserTabTitle", "Asset Browser"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "ContentBrowser.TabIcon"));

	InTabManager->RegisterTabSpawner(UnLive2DAnimationBlueprintEditorTabs::GraphDocumentTab, FOnSpawnTab::CreateSP(this,&FUnLive2DAnimationBlueprintEditor::SpawnTab_GraphDocument))
		.SetDisplayName(LOCTEXT("AnimationBlueprintTitle", "Animation Blueprint"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FUnLive2DEditorStyle::GetStyleSetName(), "ClassIcon.UnLive2DAnimBlueprint"));

	InTabManager->RegisterTabSpawner(UnLive2DAnimationBlueprintEditorTabs::PropertiesTab, FOnSpawnTab::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::SpawnTab_Properties))
		.SetDisplayName(LOCTEXT("DetailsTab", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));
}

void FUnLive2DAnimationBlueprintEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(UnLive2DAnimationBlueprintEditorTabs::ViewportTab);
	InTabManager->UnregisterTabSpawner(UnLive2DAnimationBlueprintEditorTabs::AssetBrowserTab);
	InTabManager->UnregisterTabSpawner(UnLive2DAnimationBlueprintEditorTabs::GraphDocumentTab);
	InTabManager->UnregisterTabSpawner(UnLive2DAnimationBlueprintEditorTabs::PropertiesTab);
}


void FUnLive2DAnimationBlueprintEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(UnLive2DAnimBlueprintEdited);
}

class UUnLive2DAnimBlueprint* FUnLive2DAnimationBlueprintEditor::GetBlueprintObj() const
{
	return UnLive2DAnimBlueprintEdited;
}

void FUnLive2DAnimationBlueprintEditor::BindCommands()
{
	const FUnLive2DAnimBlueprintEditorCommands& Commands = FUnLive2DAnimBlueprintEditorCommands::Get();

	ToolkitCommands->MapAction(
		Commands.PlayUnLive2DAnimBlueprint,
		FExecuteAction::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::PlayUnLive2DAnimBlueprint));
}

void FUnLive2DAnimationBlueprintEditor::SyncInBrowser()
{
	TArray<UObject*> ObjectsToSync;
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		UUnLive2DAnimBlueprintGraphNode* SelectedNode = Cast<UUnLive2DAnimBlueprintGraphNode>(*NodeIt);

		if (SelectedNode == nullptr) continue;

		UUnLive2DAnimBlueprintNode_MotionPlayer* SelectedMotion = Cast<UUnLive2DAnimBlueprintNode_MotionPlayer>(SelectedNode->AnimBlueprintNode);
		if (SelectedMotion && SelectedMotion->GetUnLive2DMotion())
		{
			ObjectsToSync.Add(SelectedMotion->GetUnLive2DMotion());
		}
	}

	if (ObjectsToSync.Num() > 0)
	{
		GEditor->SyncBrowserToObjects(ObjectsToSync);
	}
}

bool FUnLive2DAnimationBlueprintEditor::CanSyncInBrowser() const
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		UUnLive2DAnimBlueprintGraphNode* SelectedNode = Cast<UUnLive2DAnimBlueprintGraphNode>(*NodeIt);

		if (SelectedNode == nullptr) continue;

		UUnLive2DAnimBlueprintNode_MotionPlayer* SelectedMotion = Cast<UUnLive2DAnimBlueprintNode_MotionPlayer>(SelectedNode->AnimBlueprintNode);
		if (SelectedMotion && SelectedMotion->GetUnLive2DMotion())
		{
			return true;
		}
	}

	return false;
}

TSharedRef<SDockTab> FUnLive2DAnimationBlueprintEditor::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		[
			SNew(SUnLive2DAnimBlueprintEditorViewport)
			.UnLive2DAnimBlueprintEdited(this, &FUnLive2DAnimationBlueprintEditor::GetBlueprintObj)
		];
}

TSharedRef<SDockTab> FUnLive2DAnimationBlueprintEditor::SpawnTab_AssetBrowser(const FSpawnTabArgs& Args)
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	FAssetPickerConfig AssetPickerConfig;
	AssetPickerConfig.Filter.ClassNames.Add(UUnLive2DMotion::StaticClass()->GetFName());
	AssetPickerConfig.OnAssetDoubleClicked = FOnAssetDoubleClicked::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::AssetBrowser_OnMotionDoubleClicked);
	AssetPickerConfig.OnShouldFilterAsset = FOnShouldFilterAsset::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::AssetBrowser_FilterMotionBasedOnParentClass);
	AssetPickerConfig.bAllowNullSelection = true;
	AssetPickerConfig.InitialAssetViewType = EAssetViewType::Column;

	//AssetPickerConfig.SyncToAssetsDelegates.Add(&SyncToAssetsDelegate);

	return SNew(SDockTab)
		[
			ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
		];
}

TSharedRef<SDockTab> FUnLive2DAnimationBlueprintEditor::SpawnTab_GraphDocument(const FSpawnTabArgs& Args)
{
	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab);

	if (UnLive2DAnimBlueprintGraphEditor.IsValid())
	{
		SpawnedTab->SetContent(UnLive2DAnimBlueprintGraphEditor.ToSharedRef());
	}

	return SpawnedTab;
}

TSharedRef<SDockTab> FUnLive2DAnimationBlueprintEditor::SpawnTab_Properties(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
		.Label(LOCTEXT("SoundCueDetailsTitle", "Details"))
		[
			UnLive2DAnimProperties.ToSharedRef()
		];
}

TSharedRef<SGraphEditor> FUnLive2DAnimationBlueprintEditor::CreateGraphEditorWidget()
{
	if (!GraphEditorCommands.IsValid())
	{
		GraphEditorCommands = MakeShareable(new FUICommandList);

		GraphEditorCommands->MapAction(FUnLive2DAnimBlueprintEditorCommands::Get().PlayNode, // 播放节点
			FExecuteAction::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::PlayAnimNode),
			FCanExecuteAction::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::CanPlayAnimNode)
		);

		GraphEditorCommands->MapAction(FUnLive2DAnimBlueprintEditorCommands::Get().BrowserSync, // 内容浏览
			FExecuteAction::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::SyncInBrowser),
			FCanExecuteAction::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::CanSyncInBrowser)
		);

		GraphEditorCommands->MapAction(FUnLive2DAnimBlueprintEditorCommands::Get().DeleteInput, // 删除一个输入节点
			FExecuteAction::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::DeleteInput),
			FCanExecuteAction::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::CanDeleteInput)
			);

		GraphEditorCommands->MapAction(FGenericCommands::Get().Delete, // 删除节点的事件绑定
			FExecuteAction::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::DeleteSelectedNodes),
			FCanExecuteAction::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::CanDeleteNodes)
		);

		GraphEditorCommands->MapAction(FGenericCommands::Get().SelectAll, // 选择所有节点
			FExecuteAction::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::SelectAllNodes),
			FCanExecuteAction::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::CanSelectAllNodes)
		);

		GraphEditorCommands->MapAction(FGenericCommands::Get().Copy, // 复制节点数据
			FExecuteAction::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::CopySelectedNodes),
			FCanExecuteAction::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::CanCopyNodes)
		);

		GraphEditorCommands->MapAction(FGenericCommands::Get().Cut, // 剪切节点
			FExecuteAction::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::CutSelectedNodes),
			FCanExecuteAction::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::CanCutNodes)
		);

		GraphEditorCommands->MapAction(FGenericCommands::Get().Paste, // 粘贴节点
			FExecuteAction::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::PasteNodes),
			FCanExecuteAction::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::CanPasteNodes)
		);

		GraphEditorCommands->MapAction(FGenericCommands::Get().Duplicate, // 复制粘贴节点
			FExecuteAction::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::DuplicateNodes),
			FCanExecuteAction::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::CanDuplicateNodes)
		);
	}

	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText_AnimationBlueprint", "ANIMATION BLUEPRINT");

	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::OnSelectedNodesChanged);
	InEvents.OnTextCommitted = FOnNodeTextCommitted::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::OnNodeTitleCommitted);
	InEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::PlaySingleNode);

	return  SNew(SGraphEditor)
		.AdditionalCommands(GraphEditorCommands)
		.IsEditable(true)
		.Appearance(AppearanceInfo)
		.GraphToEdit(UnLive2DAnimBlueprintEdited->GetGraph())
		.GraphEvents(InEvents)
		.AutoExpandActionMenu(true)
		.ShowGraphStateOverlay(false);
}

void FUnLive2DAnimationBlueprintEditor::CreateInternalWidgets()
{
	UnLive2DAnimBlueprintGraphEditor = CreateGraphEditorWidget();

	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.NotifyHook = this;

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	UnLive2DAnimProperties = PropertyModule.CreateDetailView(Args);
	UnLive2DAnimProperties->SetObject(UnLive2DAnimBlueprintEdited);
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

	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateLambda([this](FToolBarBuilder& ToolbarBuilder)
	{
		ToolbarBuilder.BeginSection("Toolbar");
		{
			ToolbarBuilder.AddToolBarButton(FUnLive2DAnimBlueprintEditorCommands::Get().PlayUnLive2DAnimBlueprint);

			ToolbarBuilder.AddToolBarButton(FUnLive2DAnimBlueprintEditorCommands::Get().PlayNode);

			ToolbarBuilder.AddToolBarButton(FUnLive2DAnimBlueprintEditorCommands::Get().StopUnLive2DAnim);
		}
		ToolbarBuilder.EndSection();
	}));

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

void FUnLive2DAnimationBlueprintEditor::DeleteInput()
{
	if (!UnLive2DAnimBlueprintGraphEditor.IsValid()) return;

	UEdGraphPin* SelectedPin = UnLive2DAnimBlueprintGraphEditor->GetGraphPinForMenu();
	if (ensure(SelectedPin))
	{
		UUnLive2DAnimBlueprintGraphNode* SelectedNode = Cast<UUnLive2DAnimBlueprintGraphNode>(SelectedPin->GetOwningNode());

		if (SelectedNode && SelectedNode == SelectedPin->GetOwningNode())
		{
			SelectedNode->RemoveInputPin(SelectedPin);
		}
	}

}

void FUnLive2DAnimationBlueprintEditor::DeleteSelectedNodes()
{
	const FScopedTransaction Transaction( NSLOCTEXT("UnrealEd", "SoundCueEditorDeleteSelectedNode", "Delete Selected Sound Cue Node") );

	UnLive2DAnimBlueprintGraphEditor->GetCurrentGraph()->Modify();

	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	UnLive2DAnimBlueprintGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		UEdGraphNode* Node = CastChecked<UEdGraphNode>(*NodeIt);

		if (!Node->CanUserDeleteNode()) continue;

		if (UUnLive2DAnimBlueprintGraphNode* AnimBlueprintGraphNode = Cast<UUnLive2DAnimBlueprintGraphNode>(Node))
		{
			UUnLive2DAnimBlueprintNode_Base* DelNode = AnimBlueprintGraphNode->AnimBlueprintNode;

			FBlueprintEditorUtils::RemoveNode(NULL, AnimBlueprintGraphNode, true);

			// 确保已更新匹配图表
			UnLive2DAnimBlueprintEdited->CompileUnLive2DAnimNodesFromGraphNodes();

			// 从UnLive2DAnimBlueprint的所有UnLive2DAnimBlueprintNodes列表中删除此节点
			UnLive2DAnimBlueprintEdited->GetGraphAllNodes().Remove(DelNode);
			UnLive2DAnimBlueprintEdited->MarkPackageDirty();
		}
		else
		{
			FBlueprintEditorUtils::RemoveNode(NULL, Node, true);
		}
	}
}

bool FUnLive2DAnimationBlueprintEditor::CanDeleteNodes() const
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	if (SelectedNodes.Num() == 1)
	{
		for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
		{
			if (Cast<UUnLive2DAnimBlueprintGraphNode_Root>(*NodeIt))
			{
				return false;
			}
			return true;
		}
	}

	return SelectedNodes.Num() > 0;
}

void FUnLive2DAnimationBlueprintEditor::DeleteSelectedDuplicatableNodes()
{
	// 缓存旧选择
	const FGraphPanelSelectionSet OldSelectedNodes = GetSelectedNodes();

	// 清除选择并仅选择可以复制的节点
	UnLive2DAnimBlueprintGraphEditor->ClearSelectionSet();

	FGraphPanelSelectionSet RemainingNodes;
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(OldSelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if ((Node != NULL) && Node->CanDuplicateNode())
		{
			UnLive2DAnimBlueprintGraphEditor->SetNodeSelection(Node, true);
		}
		else
		{
			RemainingNodes.Add(Node);
		}
	}

	// 删除可复制的节点
	DeleteSelectedNodes();

	UnLive2DAnimBlueprintGraphEditor->ClearSelectionSet();

	// 还原选中但无法复制的节点
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(RemainingNodes); SelectedIter; ++SelectedIter)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter))
		{
			UnLive2DAnimBlueprintGraphEditor->SetNodeSelection(Node, true);
		}
	}
}

void FUnLive2DAnimationBlueprintEditor::SelectAllNodes()
{
	UnLive2DAnimBlueprintGraphEditor->SelectAllNodes();
}

void FUnLive2DAnimationBlueprintEditor::CopySelectedNodes()
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		if (UUnLive2DAnimBlueprintGraphNode* Node = Cast<UUnLive2DAnimBlueprintGraphNode>(*NodeIt))
		{
			Node->PrepareForCopying();
		}
	}

	FString ExportedText;
	FEdGraphUtilities::ExportNodesToText(SelectedNodes, /*out*/ ExportedText);
	FPlatformApplicationMisc::ClipboardCopy(*ExportedText);


	// 返回复制后的所有权数据
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		if (UUnLive2DAnimBlueprintGraphNode* Node = Cast<UUnLive2DAnimBlueprintGraphNode>(*SelectedIter))
		{
			Node->PostCopyNode();
		}
	}
}

bool FUnLive2DAnimationBlueprintEditor::CanCopyNodes() const
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*NodeIt);
		if ((Node != NULL) && Node->CanDuplicateNode())
		{
			return true;
		}
	}

	return false;
}

void FUnLive2DAnimationBlueprintEditor::CutSelectedNodes()
{
	CopySelectedNodes();
	DeleteSelectedDuplicatableNodes();
}

bool FUnLive2DAnimationBlueprintEditor::CanCutNodes() const
{
	return CanCopyNodes() && CanDeleteNodes();
}

void FUnLive2DAnimationBlueprintEditor::PasteNodes()
{
	PasteNodesHere(UnLive2DAnimBlueprintGraphEditor->GetPasteLocation());
}

void FUnLive2DAnimationBlueprintEditor::DuplicateNodes()
{
	CopySelectedNodes();
	PasteNodes();
}

bool FUnLive2DAnimationBlueprintEditor::CanDuplicateNodes() const
{
	return CanCopyNodes();
}

void FUnLive2DAnimationBlueprintEditor::PlayUnLive2DAnimBlueprint()
{
	if (UUnLive2DAnimBlueprintNode_MotionPlayer* SelectedMotion = Cast<UUnLive2DAnimBlueprintNode_MotionPlayer>(UnLive2DAnimBlueprintEdited->FirstNode))
	{
		if (SelectedMotion->GetUnLive2DMotion() == nullptr) return;

		SelectedMotion->GetUnLive2DMotion()->PlayMotion();
	}
	
}

void FUnLive2DAnimationBlueprintEditor::PlayAnimNode()
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		PlaySingleNode(CastChecked<UEdGraphNode>(*NodeIt));
	}
}

bool FUnLive2DAnimationBlueprintEditor::CanPlayAnimNode() const
{
	return GetSelectedNodes().Num() == 1;
}

void FUnLive2DAnimationBlueprintEditor::OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection)
{
	TArray<UObject*> Selection;

	if (NewSelection.Num())
	{
		for (TSet<class UObject*>::TConstIterator SetIt(NewSelection); SetIt; ++SetIt)
		{
			if (Cast<UUnLive2DAnimBlueprintGraphNode_Root>(*SetIt))
			{
				Selection.Add(GetBlueprintObj());
			}
			else if (UUnLive2DAnimBlueprintGraphNode* GraphNode =  Cast<UUnLive2DAnimBlueprintGraphNode>(*SetIt))
			{
				Selection.Add(GraphNode->AnimBlueprintNode);
			}
			else
			{
				Selection.Add(*SetIt);
			}
		}
	}
	else
	{
		Selection.Add(GetBlueprintObj());
	}
	SetSelection(Selection);
}

void FUnLive2DAnimationBlueprintEditor::OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged)
{
	if (NodeBeingChanged)
	{
		const FScopedTransaction Transaction(LOCTEXT("RenameNode", "Rename Node"));
		NodeBeingChanged->Modify();
		NodeBeingChanged->OnRenameNode(NewText.ToString());
	}
}

void FUnLive2DAnimationBlueprintEditor::PlaySingleNode(UEdGraphNode* Node)
{
	UUnLive2DAnimBlueprintGraphNode* UnLive2DAnimBlueprintGraphNode = Cast<UUnLive2DAnimBlueprintGraphNode>(Node);

	if (UnLive2DAnimBlueprintGraphNode)
	{
		if (UUnLive2DAnimBlueprintNode_MotionPlayer* SelectedMotion = Cast<UUnLive2DAnimBlueprintNode_MotionPlayer>(UnLive2DAnimBlueprintGraphNode->AnimBlueprintNode))
		{
			if (SelectedMotion->GetUnLive2DMotion() == nullptr) return;

			SelectedMotion->GetUnLive2DMotion()->PlayMotion();
			
		}
	}
}

void FUnLive2DAnimationBlueprintEditor::AssetBrowser_OnMotionDoubleClicked(const FAssetData& AssetData) const
{
	if (UObject* RawAsset = AssetData.GetAsset())
	{
		if (UUnLive2DMotion* Motion = Cast<UUnLive2DMotion>(RawAsset))
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Motion);
		}
	}
}

bool FUnLive2DAnimationBlueprintEditor::AssetBrowser_FilterMotionBasedOnParentClass(const FAssetData& AssetData) const
{
	if (!AssetData.IsValid()) return true;

	UUnLive2DMotion* TargetUnLive2DMotion = Cast<UUnLive2DMotion>(AssetData.GetAsset());

	if (UUnLive2DAnimBlueprint* AnimBlueprintPtr = GetBlueprintObj())
	{
		if (TargetUnLive2DMotion)
		{
			return !(TargetUnLive2DMotion->UnLive2D == AnimBlueprintPtr->TargetUnLive2D);
		}
	}

	return true;
}

void FUnLive2DAnimationBlueprintEditor::InitUnLive2DAnimationBlueprintEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, class UUnLive2DAnimBlueprint* InAnimBlueprint)
{
	UnLive2DAnimBlueprintEdited = InAnimBlueprint;

	UnLive2DAnimBlueprintEdited->SetFlags(RF_Transactional);

	GEditor->RegisterForUndo(this);

	FGraphEditorCommands::Register();
	FUnLive2DAnimBlueprintEditorCommands::Register();

	FUnLive2DManagerModule& MangerModule = FModuleManager::LoadModuleChecked<FUnLive2DManagerModule>("UnLive2DManager");
	UnLive2DManagerToolkit = MangerModule.CreatePersonaToolkit(InAnimBlueprint);

	TSharedRef<IUnLive2DAssetFamily> AssetFamily = MangerModule.CreatePersonaAssetFamily(InAnimBlueprint);
	AssetFamily->RecordAssetOpened(FAssetData(InAnimBlueprint));

	// Build up a list of objects being edited in this asset editor

	TSharedPtr<FUnLive2DAnimationBlueprintEditor> ThisPtr(SharedThis(this));
	if (!DocumentManager.IsValid())
	{
		DocumentManager = MakeShareable(new FDocumentTracker);
		DocumentManager->Initialize(ThisPtr);
	}

	TArray<UObject*> AnimBlueprints;
	AnimBlueprints.Add(InAnimBlueprint);

	BindCommands();

	CreateInternalWidgets();

	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Stanalone_UnLive2DAnimBlueprintEditorMode_Layout_v1.04")
	->AddArea
	(
		{
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				// Top toolbar
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->SetHideTabWell(true)
#if UE_VERSION_OLDER_THAN(5,0,0)
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
#else
				->AddTab(GetEditorName(), ETabState::OpenedTab)
#endif
			)
			->Split
			(
				{
					// Main application area
					FTabManager::NewSplitter()
					->SetOrientation(Orient_Horizontal)
					->Split
					(
						// Left top - viewport
						{
							FTabManager::NewSplitter()
							->SetSizeCoefficient(0.25f)
							->SetOrientation(Orient_Vertical)
							->Split
							(
								{
									// Left top - viewport
									FTabManager::NewStack()
									->SetSizeCoefficient(0.5f)
									->SetHideTabWell(true)
									->AddTab(UnLive2DAnimationBlueprintEditorTabs::ViewportTab, ETabState::OpenedTab)
								}
							)
							/*->Split
							(
								{
									//	Left bottom - preview settings
									FTabManager::NewStack()
									->SetSizeCoefficient(0.5f)
								//->AddTab(UnLive2DAnimationBlueprintEditorTabs::CurveNamesTab, ETabState::OpenedTab)
								->AddTab(FBlueprintEditorTabs::MyBlueprintID, ETabState::OpenedTab)
							}
							)*/
						}
					)
					->Split
					(
						// Middle 
						{
							FTabManager::NewSplitter()
							->SetOrientation(Orient_Vertical)
							->SetSizeCoefficient(0.55f)
							->Split
							(
								// Middle top - document edit area
								FTabManager::NewStack()
								->SetSizeCoefficient(0.8f)
								->SetHideTabWell(true)
								->AddTab(UnLive2DAnimationBlueprintEditorTabs::GraphDocumentTab, ETabState::OpenedTab)
							)
							/*->Split
							(
								// Middle bottom - compiler results & find
								FTabManager::NewStack()
								->SetSizeCoefficient(0.2f)
								->AddTab(FBlueprintEditorTabs::CompilerResultsID, ETabState::ClosedTab)
								->AddTab(FBlueprintEditorTabs::FindResultsID, ETabState::ClosedTab)
							)*/
						}
					)
					->Split
					(
						// Right side
						{
							FTabManager::NewSplitter()
							->SetSizeCoefficient(0.2f)
							->SetOrientation(Orient_Vertical)
							->Split
							(
								// Right top - selection details panel & overrides
								FTabManager::NewStack()
								->SetHideTabWell(true)
								->SetSizeCoefficient(0.5f)
								->AddTab(UnLive2DAnimationBlueprintEditorTabs::PropertiesTab, ETabState::OpenedTab)
							)
							->Split
							(
								// Right bottom - Asset browser & advanced preview settings
								FTabManager::NewStack()
								->SetHideTabWell(true)
								->SetSizeCoefficient(0.5f)
								->AddTab(UnLive2DAnimationBlueprintEditorTabs::AssetBrowserTab, ETabState::OpenedTab)
							)
						}
					)
				}
			)
		}
	);


	const FName UnLive2DAnimationBlueprintEditorAppName(TEXT("UnLive2DAnimationBlueprintEditorApp"));

	// Initialize the asset editor and spawn tabs
	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;
	InitAssetEditor(Mode, InitToolkitHost, UnLive2DAnimationBlueprintEditorAppName, StandaloneDefaultLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, InAnimBlueprint);

	ExtendMenu();
	ExtendToolbar();
	RegenerateMenusAndToolbars();

}

#undef LOCTEXT_NAMESPACE