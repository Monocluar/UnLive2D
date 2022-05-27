#include "UnLive2DViewEditor.h"
#include "SScrubControlPanel.h"
#include "UnLive2DEditorViewport.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "UnLive2D.h"
#include "SSingleObjectDetailsPanel.h"
#include "UnLive2DManagerModule.h"
#include "IUnLive2DAssetFamily.h"
#include "Misc/EngineVersionComparison.h"


#define LOCTEXT_NAMESPACE "FUnLive2DAssetEditorModule"

class SUnLive2DPropertiesTabBody : public SSingleObjectDetailsPanel
{
public:
	SLATE_BEGIN_ARGS(SUnLive2DPropertiesTabBody) {}
	SLATE_END_ARGS()

private:

	TWeakPtr<class FUnLive2DViewEditor> UnLive2DEditorPtr;

public:

	void Construct(const FArguments& InArgs, TSharedPtr<FUnLive2DViewEditor> InUnLive2DEditor)
	{
		UnLive2DEditorPtr = InUnLive2DEditor;

		SSingleObjectDetailsPanel::Construct(SSingleObjectDetailsPanel::FArguments().HostCommandList(InUnLive2DEditor->GetToolkitCommands()).HostTabManager(InUnLive2DEditor->GetTabManager()), /*bAutomaticallyObserveViaGetObjectToObserve=*/ true, /*bAllowSearch=*/ true);
	}

	virtual UObject* GetObjectToObserve() const override
	{
		return UnLive2DEditorPtr.Pin()->GetUnLive2DBeingEdited();
	}

	virtual TSharedRef<SWidget> PopulateSlot(TSharedRef<SWidget> PropertyEditorWidget) override
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.FillHeight(1)
			[
				PropertyEditorWidget
			];
	}
};

struct FUnLive2DViewEditorTabs
{
	// Tab identifiers
	static const FName DetailsID;
	static const FName ViewportID;
};

const FName FUnLive2DViewEditorTabs::DetailsID(TEXT("Details"));
const FName FUnLive2DViewEditorTabs::ViewportID(TEXT("Viewport"));

FUnLive2DViewEditor::FUnLive2DViewEditor()
	:UnLive2DBeingEdited(nullptr)
{
}

FString FUnLive2DViewEditor::GetReferencerName() const
{
	return TEXT("FUnLive2DViewEditor");
}

void FUnLive2DViewEditor::InitUnLive2DViewEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, UUnLive2D* InitUnLive2D)
{
	//GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseOtherEditors(InitUnLive2D, this); // 关闭其他的Live2D编辑器

	UnLive2DBeingEdited = InitUnLive2D;

	FUnLive2DManagerModule& MangerModule = FModuleManager::LoadModuleChecked<FUnLive2DManagerModule>("UnLive2DManager");
	UnLive2DToolkit = MangerModule.CreatePersonaToolkit(UnLive2DBeingEdited);

	TSharedRef<IUnLive2DAssetFamily> AssetFamily = MangerModule.CreatePersonaAssetFamily(UnLive2DBeingEdited);
	AssetFamily->RecordAssetOpened(FAssetData(UnLive2DBeingEdited));

	const FName UnLive2DEditorAppName = FName(TEXT("UnLive2DEditorApp"));

	BindCommands();

	ViewportPtr = SNew(SUnLive2DEditorViewport)
		.UnLive2DBeingEdited(this, &FUnLive2DViewEditor::GetUnLive2DBeingEdited);

	// Default layout Standalone_FlipbookEditor_Layout_v1
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_UnLive2DViewEditor_Layout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
#if UE_VERSION_OLDER_THAN(5,0,0)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->SetHideTabWell(true)
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
			)
#endif
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(0.9f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.8f)
					->SetHideTabWell(true)
					->AddTab(FUnLive2DViewEditorTabs::ViewportID, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.2f)
					->AddTab(FUnLive2DViewEditorTabs::DetailsID, ETabState::OpenedTab)
				)
			)
		);

	InitAssetEditor(Mode, InitToolkitHost, UnLive2DEditorAppName, StandaloneDefaultLayout, /*bCreateDefaultStandaloneMenu=*/ true, /*bCreateDefaultToolbar=*/ true, InitUnLive2D);

	ExtendToolbar();

	RegenerateMenusAndToolbars();
}

void FUnLive2DViewEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_UnLive2DViewEdito", "UnLive2DView Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FUnLive2DViewEditorTabs::ViewportID, FOnSpawnTab::CreateSP(this, &FUnLive2DViewEditor::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("ViewportTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));

	InTabManager->RegisterTabSpawner(FUnLive2DViewEditorTabs::DetailsID, FOnSpawnTab::CreateSP(this, &FUnLive2DViewEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTabLabel", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));
}

void FUnLive2DViewEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(FUnLive2DViewEditorTabs::ViewportID);
	InTabManager->UnregisterTabSpawner(FUnLive2DViewEditorTabs::DetailsID);
}

void FUnLive2DViewEditor::PostUndo(bool bSuccess)
{
	OnPostUndo.Broadcast();
}

void FUnLive2DViewEditor::PostRedo(bool bSuccess)
{
	OnPostUndo.Broadcast();
}

bool FUnLive2DViewEditor::OnRequestClose()
{
	bool bAllowClose = true;

	if (UnLive2DToolkit.IsValid() && UnLive2DBeingEdited)
	{

	}

	return bAllowClose;
}

FName FUnLive2DViewEditor::GetToolkitFName() const
{
	return FName("UnLive2DEditor");
}

FText FUnLive2DViewEditor::GetBaseToolkitName() const
{
	return LOCTEXT("UnLive2DEditorAppLabel", "UnLive2D Editor");
}

FText FUnLive2DViewEditor::GetToolkitName() const
{
	return FText::FromString(UnLive2DBeingEdited->GetName());
}

FText FUnLive2DViewEditor::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(UnLive2DBeingEdited);
}

FLinearColor FUnLive2DViewEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

FString FUnLive2DViewEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("UnLive2DAssetEditor");
}

FString FUnLive2DViewEditor::GetDocumentationLink() const
{
	return TEXT("Engine/UnLive2DAsset/UnLive2DEditor");
}

void FUnLive2DViewEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(UnLive2DBeingEdited);
}

TSharedRef<SDockTab> FUnLive2DViewEditor::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	ViewInputMin = 0.0f;
	ViewInputMax = GetTotalSequenceLength();
	LastObservedSequenceLength = ViewInputMax;

	// 序列播放管理器
	/*TSharedRef<SWidget> ScrubControl = SNew(SScrubControlPanel)
		.IsEnabled(true)
		.Value(this, &FUnLive2DViewEditor::GetPlaybackPosition)
		.NumOfKeys(this, &FUnLive2DViewEditor::GetTotalFrameCountPlusOne)
		.SequenceLength(this, &FUnLive2DViewEditor::GetTotalSequenceLength)
		.OnValueChanged(this, &FUnLive2DViewEditor::SetPlaybackPosition)
		//		.OnBeginSliderMovement(this, &FUnLive2DViewEditor::OnBeginSliderMovement)
		//		.OnEndSliderMovement(this, &FUnLive2DViewEditor::OnEndSliderMovement)
		.OnClickedForwardPlay(this, &FUnLive2DViewEditor::OnClick_Forward)
		.OnClickedForwardStep(this, &FUnLive2DViewEditor::OnClick_Forward_Step)
		.OnClickedForwardEnd(this, &FUnLive2DViewEditor::OnClick_Forward_End)
		.OnClickedBackwardPlay(this, &FUnLive2DViewEditor::OnClick_Backward)
		.OnClickedBackwardStep(this, &FUnLive2DViewEditor::OnClick_Backward_Step)
		.OnClickedBackwardEnd(this, &FUnLive2DViewEditor::OnClick_Backward_End)
		.OnClickedToggleLoop(this, &FUnLive2DViewEditor::OnClick_ToggleLoop)
		.OnGetLooping(this, &FUnLive2DViewEditor::IsLooping)
		.OnGetPlaybackMode(this, &FUnLive2DViewEditor::GetPlaybackMode)
		.ViewInputMin(this, &FUnLive2DViewEditor::GetViewRangeMin)
		.ViewInputMax(this, &FUnLive2DViewEditor::GetViewRangeMax)
		.OnSetInputViewRange(this, &FUnLive2DViewEditor::SetViewRange)
		.bAllowZoom(true)
		.IsRealtimeStreamingMode(false);*/

	return SNew(SDockTab)
		.Label(LOCTEXT("ViewportTab_Title", "Viewport"))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			[
				ViewportPtr.ToSharedRef()
			]

			/*+ SVerticalBox::Slot()
				.Padding(0, 8, 0, 0)
				.AutoHeight()
				[
					SNew(SFlipbookTimeline, GetToolkitCommands())
					.FlipbookBeingEdited(this, &FFlipbookEditor::GetFlipbookBeingEdited)
				.OnSelectionChanged(this, &FFlipbookEditor::SetSelection)
				.PlayTime(this, &FFlipbookEditor::GetPlaybackPosition)
				]

			+ SVerticalBox::Slot()
				.Padding(0, 8, 0, 0)
				.AutoHeight()
				[
					ScrubControl
				]*/
		];
}

TSharedRef<SDockTab> FUnLive2DViewEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		//.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
		.Label(LOCTEXT("DetailsTab_Title", "Details"))
		[
			SNew(SUnLive2DPropertiesTabBody, SharedThis(this))
		];
}

float FUnLive2DViewEditor::GetTotalSequenceLength() const
{
	return 200.f;
}

void FUnLive2DViewEditor::BindCommands()
{
}

void FUnLive2DViewEditor::ExtendToolbar()
{
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
		TSharedRef<IUnLive2DAssetFamily> AssetFamily = MangerModule.CreatePersonaAssetFamily(UnLive2DBeingEdited);
		AddToolbarWidget(MangerModule.CreateAssetFamilyShortcutWidget(SharedThis(this), AssetFamily));
	}));
}


#undef LOCTEXT_NAMESPACE