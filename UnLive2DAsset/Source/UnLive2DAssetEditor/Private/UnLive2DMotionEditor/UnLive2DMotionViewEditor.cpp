#include "UnLive2DMotionViewEditor.h"
#include "UnLive2DManagerModule.h"
#include "SSingleObjectDetailsPanel.h"
#include "IUnLive2DAssetFamily.h"
#include "UnLive2DMotion.h"
#include "UnLive2DMotionEditor/SUnLive2DMotionAssetBrowser.h"
#include "SUnLive2DMotionEditorViewport.h"
#include "UnLive2DMotionViewportClient.h"
#include "UnLive2D.h"
#include "Misc/EngineVersionComparison.h"

#define LOCTEXT_NAMESPACE "FUnLive2DAssetEditorModule"

struct FUnLive2DMotionViewEditorTabs
{
	// Tab identifiers
	static const FName DetailsID;
	static const FName ViewportID;
	static const FName AssetBrowserTab;
};

const FName FUnLive2DMotionViewEditorTabs::DetailsID(TEXT("Details"));
const FName FUnLive2DMotionViewEditorTabs::ViewportID(TEXT("Viewport"));
const FName FUnLive2DMotionViewEditorTabs::AssetBrowserTab(TEXT("AssetBrowserTab"));

class SUnLive2DMotionPropertiesTabBody : public SSingleObjectDetailsPanel
{
public:
	SLATE_BEGIN_ARGS(SUnLive2DMotionPropertiesTabBody) {}
	SLATE_END_ARGS()

private:

	TWeakPtr<class FUnLive2DMotionViewEditor> UnLive2DMotionEditorPtr;

public:

	void Construct(const FArguments& InArgs, TSharedPtr<FUnLive2DMotionViewEditor> InUnLive2DMotionEditor)
	{
		UnLive2DMotionEditorPtr = InUnLive2DMotionEditor;

		SSingleObjectDetailsPanel::Construct(SSingleObjectDetailsPanel::FArguments().HostCommandList(InUnLive2DMotionEditor->GetToolkitCommands()).HostTabManager(InUnLive2DMotionEditor->GetTabManager()), /*bAutomaticallyObserveViaGetObjectToObserve=*/ true, /*bAllowSearch=*/ true);
	}

	virtual UObject* GetObjectToObserve() const override
	{
		return UnLive2DMotionEditorPtr.Pin()->GetUnLive2DMotionEdited();
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

FUnLive2DMotionViewEditor::FUnLive2DMotionViewEditor()
	: UnLive2DMotionBeingEdited(nullptr)
{

}

void FUnLive2DMotionViewEditor::InitUnLive2DMotionViewEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, UUnLive2DMotion* InitUnLive2DMotion)
{
	UnLive2DMotionBeingEdited = InitUnLive2DMotion;
	FUnLive2DManagerModule& MangerModule = FModuleManager::LoadModuleChecked<FUnLive2DManagerModule>("UnLive2DManager");
	UnLive2DMotionToolkit = MangerModule.CreatePersonaToolkit(UnLive2DMotionBeingEdited);

	TSharedRef<IUnLive2DAssetFamily> AssetFamily = MangerModule.CreatePersonaAssetFamily(InitUnLive2DMotion);
	AssetFamily->RecordAssetOpened(FAssetData(InitUnLive2DMotion));

	UpDataMotion();

	BindCommands();

	ViewportPtr = SNew(SUnLive2DMotionEditorViewport)
		.UnLive2DMotionBeingEdited(this, &FUnLive2DMotionViewEditor::GetUnLive2DMotionEdited);

	UnLive2DMotionAssetListPtr = SNew(SUnLive2DMotionAssetBrowser, SharedThis(this));

	const FName MotionViewEditorAppIdentifier = FName(TEXT("MotionViewEditorApp"));

	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_UnLive2DMotionViewEditor_Layout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
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
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(0.9f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.8f)
					->SetHideTabWell(true)
					->AddTab(FUnLive2DMotionViewEditorTabs::ViewportID, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewSplitter()
					->SetSizeCoefficient(0.2f)
					->SetOrientation(Orient_Vertical)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.6f)
						->AddTab(FUnLive2DMotionViewEditorTabs::DetailsID, ETabState::OpenedTab)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.4f)
						->AddTab(FUnLive2DMotionViewEditorTabs::AssetBrowserTab, ETabState::OpenedTab)
					)
				)
			)
		);

	InitAssetEditor(Mode, InitToolkitHost, MotionViewEditorAppIdentifier, StandaloneDefaultLayout, /*bCreateDefaultStandaloneMenu=*/ true, /*bCreateDefaultToolbar=*/ true, InitUnLive2DMotion);

	ExtendToolbar();

	RegenerateMenusAndToolbars();
}

void FUnLive2DMotionViewEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_UnLive2DMotionViewEditorr", "UnLive2DView Motion Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FUnLive2DMotionViewEditorTabs::ViewportID, FOnSpawnTab::CreateSP(this, &FUnLive2DMotionViewEditor::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("ViewportTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));

	InTabManager->RegisterTabSpawner(FUnLive2DMotionViewEditorTabs::DetailsID, FOnSpawnTab::CreateSP(this, &FUnLive2DMotionViewEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTabLabel", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(FUnLive2DMotionViewEditorTabs::AssetBrowserTab, FOnSpawnTab::CreateSP(this, &FUnLive2DMotionViewEditor::SpawnTab_AssetBrowser))
		.SetDisplayName(LOCTEXT("AssetBrowserTab", "AssetBrowser"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.ContentBrowser"));
}

void FUnLive2DMotionViewEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(FUnLive2DMotionViewEditorTabs::ViewportID);
	InTabManager->UnregisterTabSpawner(FUnLive2DMotionViewEditorTabs::DetailsID);
	InTabManager->UnregisterTabSpawner(FUnLive2DMotionViewEditorTabs::AssetBrowserTab);
}

void FUnLive2DMotionViewEditor::PostUndo(bool bSuccess)
{
	OnPostUndo.Broadcast();
}

void FUnLive2DMotionViewEditor::PostRedo(bool bSuccess)
{
	OnPostUndo.Broadcast();
}

FName FUnLive2DMotionViewEditor::GetToolkitFName() const
{
	return FName("UnLive2DMotionViewEditor");
}

FText FUnLive2DMotionViewEditor::GetBaseToolkitName() const
{
	return LOCTEXT("UnLive2DMotionEditorAppLabel", "UnLive2D Motion Editor");
}

FText FUnLive2DMotionViewEditor::GetToolkitName() const
{
	return FText::FromString(UnLive2DMotionBeingEdited->GetName());
}

FText FUnLive2DMotionViewEditor::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(UnLive2DMotionBeingEdited);
}

FLinearColor FUnLive2DMotionViewEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

FString FUnLive2DMotionViewEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("UnLive2DMotionAssetEditor");
}

FString FUnLive2DMotionViewEditor::GetDocumentationLink() const
{
	return TEXT("Engine/UnLive2DAsset/UnLive2DMotionEditor");
}

void FUnLive2DMotionViewEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(UnLive2DMotionBeingEdited);
}

void FUnLive2DMotionViewEditor::SetUnLive2DMotionBeingEdited(UUnLive2DMotion* NewMotion)
{
	if ((NewMotion != UnLive2DMotionBeingEdited) && (NewMotion != nullptr))
	{
		UUnLive2DMotion* OldMotion = UnLive2DMotionBeingEdited;
		UnLive2DMotionBeingEdited = NewMotion;

		RemoveEditingObject(OldMotion);
		AddEditingObject(NewMotion);

		UnLive2DMotionAssetListPtr->SelectAsset(NewMotion);

		OpenNewMotionDocumentTab(NewMotion);
	}
}

void FUnLive2DMotionViewEditor::BindCommands()
{

}

void FUnLive2DMotionViewEditor::ExtendToolbar()
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
		TSharedRef<IUnLive2DAssetFamily> AssetFamily = MangerModule.CreatePersonaAssetFamily(UnLive2DMotionBeingEdited);
		AddToolbarWidget(MangerModule.CreateAssetFamilyShortcutWidget(SharedThis(this), AssetFamily));
	}));
}

void FUnLive2DMotionViewEditor::UpDataMotion()
{
	if (UnLive2DMotionBeingEdited && UnLive2DMotionBeingEdited->UnLive2D)
	{
		UnLive2DMotionBeingEdited->UnLive2D->PlayMotion(UnLive2DMotionBeingEdited);
	}
}

TSharedRef<SDockTab> FUnLive2DMotionViewEditor::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("ViewportTab_Title", "Viewport"))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			[
				ViewportPtr.ToSharedRef()
			]
		];
}

TSharedRef<SDockTab> FUnLive2DMotionViewEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		//.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
		.Label(LOCTEXT("DetailsTab_Title", "Details"))
		[
			SNew(SUnLive2DMotionPropertiesTabBody, SharedThis(this))
		];
}

TSharedRef<SDockTab> FUnLive2DMotionViewEditor::SpawnTab_AssetBrowser(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		//.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.ContentBrowser"))
		.Label(LOCTEXT("AssetBrowserTab", "AssetBrowser"))
		[
			//SNew(SUnLive2DMotionPropertiesTabBody, SharedThis(this))
			UnLive2DMotionAssetListPtr.ToSharedRef()
		];
}

TSharedPtr<SDockTab> FUnLive2DMotionViewEditor::OpenNewMotionDocumentTab(UUnLive2DMotion* InMotion)
{
	TSharedPtr<SDockTab> OpenedTab;

	if (InMotion != nullptr)
	{
		FUnLive2DManagerModule& MangerModule = FModuleManager::GetModuleChecked<FUnLive2DManagerModule>("UnLive2DManager");

		TSharedRef<IUnLive2DAssetFamily> AssetFamily = MangerModule.CreatePersonaAssetFamily(InMotion);
		AssetFamily->RecordAssetOpened(FAssetData(InMotion));

		UpDataMotion();
	}

	return OpenedTab;
}

#undef LOCTEXT_NAMESPACE