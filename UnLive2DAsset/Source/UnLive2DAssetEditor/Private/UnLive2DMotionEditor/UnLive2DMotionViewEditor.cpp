#include "UnLive2DMotionViewEditor.h"
#include "UnLive2DManagerModule.h"
#include "SSingleObjectDetailsPanel.h"
#include "IUnLive2DAssetFamily.h"
#include "Animation/UnLive2DMotion.h"
#include "UnLive2DMotionEditor/SUnLive2DMotionAssetBrowser.h"
#include "SUnLive2DMotionEditorViewport.h"
#include "UnLive2DMotionViewportClient.h"
#include "UnLive2D.h"
#include "Misc/EngineVersionComparison.h"
#include "Animation/UnLive2DExpression.h"
#include "UnLive2DViewportClient.h"
#include "UnLive2DRendererComponent.h"
#include "UnLive2DParameterEditor/SUnLive2DParameterGroup.h"
#include "UnLive2DCubismCore.h"

#define LOCTEXT_NAMESPACE "FUnLive2DAssetEditorModule"

struct FUnLive2DMotionViewEditorTabs
{
	// Tab identifiers
	static const FName DetailsID;
	static const FName ViewportID;
	static const FName AssetBrowserTab;
	static const FName ParmeterGroupID;
};

const FName FUnLive2DMotionViewEditorTabs::DetailsID(TEXT("Details"));
const FName FUnLive2DMotionViewEditorTabs::ViewportID(TEXT("Viewport"));
const FName FUnLive2DMotionViewEditorTabs::AssetBrowserTab(TEXT("AssetBrowserTab"));
const FName FUnLive2DMotionViewEditorTabs::ParmeterGroupID(TEXT("ParmeterGroupID"));

class SUnLive2DMotionPropertiesTabBody : public SSingleObjectDetailsPanel
{
public:
	SLATE_BEGIN_ARGS(SUnLive2DMotionPropertiesTabBody) {}
	SLATE_END_ARGS()

private:

	TWeakPtr<class FUnLive2DAnimBaseViewEditor> UnLive2DMotionEditorPtr;

public:

	void Construct(const FArguments& InArgs, TSharedPtr<FUnLive2DAnimBaseViewEditor> InUnLive2DMotionEditor)
	{
		UnLive2DMotionEditorPtr = InUnLive2DMotionEditor;

		SSingleObjectDetailsPanel::Construct(SSingleObjectDetailsPanel::FArguments().HostCommandList(InUnLive2DMotionEditor->GetToolkitCommands()).HostTabManager(InUnLive2DMotionEditor->GetTabManager()), /*bAutomaticallyObserveViaGetObjectToObserve=*/ true, /*bAllowSearch=*/ true);
	}

	virtual UObject* GetObjectToObserve() const override
	{
		return UnLive2DMotionEditorPtr.Pin()->GetUnLive2DAnimBaseEdited();
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

FUnLive2DAnimBaseViewEditor::FUnLive2DAnimBaseViewEditor()
	: UnLive2DAnimBeingEdited(nullptr)
{

}

FString FUnLive2DAnimBaseViewEditor::GetReferencerName() const
{
	return TEXT("UnLive2DAnimBaseViewEditor");
}

void FUnLive2DAnimBaseViewEditor::InitUnLive2DAnimViewEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, UUnLive2DAnimBase* InitUnLive2DAnimBase)
{
	UnLive2DAnimBeingEdited = InitUnLive2DAnimBase;
	FUnLive2DManagerModule& MangerModule = FModuleManager::LoadModuleChecked<FUnLive2DManagerModule>("UnLive2DManager");
	UnLive2DAnimToolkit = MangerModule.CreatePersonaToolkit(UnLive2DAnimBeingEdited);

	TSharedRef<IUnLive2DAssetFamily> AssetFamily = MangerModule.CreatePersonaAssetFamily(InitUnLive2DAnimBase);
	AssetFamily->RecordAssetOpened(FAssetData(InitUnLive2DAnimBase));


	BindCommands();

	ViewportPtr = SNew(SUnLive2DAnimBaseEditorViewport)
		.UnLive2DAnimBaseBeingEdited(this, &FUnLive2DAnimBaseViewEditor::GetUnLive2DAnimBaseEdited);

	UnLive2DAnimAssetListPtr = SNew(SUnLive2DAnimBaseAssetBrowser, SharedThis(this));

	const FName MotionViewEditorAppIdentifier = FName(TEXT("AnimBaseViewEditorApp"));

	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_UnLive2DAnimBaseViewEditor_Layout_v1.1")
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
						->AddTab(FUnLive2DMotionViewEditorTabs::ParmeterGroupID, ETabState::OpenedTab)
						->SetForegroundTab(FUnLive2DMotionViewEditorTabs::DetailsID)

					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.4f)
						->SetHideTabWell(true)
						->AddTab(FUnLive2DMotionViewEditorTabs::AssetBrowserTab, ETabState::OpenedTab)
					)
				)
			)
		);

	InitAssetEditor(Mode, InitToolkitHost, MotionViewEditorAppIdentifier, StandaloneDefaultLayout, /*bCreateDefaultStandaloneMenu=*/ true, /*bCreateDefaultToolbar=*/ true, InitUnLive2DAnimBase);

	ExtendToolbar();

	RegenerateMenusAndToolbars();

	UpDataAnimBase();
}

void FUnLive2DAnimBaseViewEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_UnLive2DMotionViewEditorr", "UnLive2DView Motion Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FUnLive2DMotionViewEditorTabs::ViewportID, FOnSpawnTab::CreateSP(this, &FUnLive2DAnimBaseViewEditor::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("ViewportTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FUnLive2DAppStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));

	InTabManager->RegisterTabSpawner(FUnLive2DMotionViewEditorTabs::DetailsID, FOnSpawnTab::CreateSP(this, &FUnLive2DAnimBaseViewEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTabLabel", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FUnLive2DAppStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(FUnLive2DMotionViewEditorTabs::AssetBrowserTab, FOnSpawnTab::CreateSP(this, &FUnLive2DAnimBaseViewEditor::SpawnTab_AssetBrowser))
		.SetDisplayName(LOCTEXT("AssetBrowserTab", "AssetBrowser"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FUnLive2DAppStyle::GetStyleSetName(), "LevelEditor.Tabs.ContentBrowser"));

	InTabManager->RegisterTabSpawner(FUnLive2DMotionViewEditorTabs::ParmeterGroupID, FOnSpawnTab::CreateSP(this, &FUnLive2DAnimBaseViewEditor::SpawnTab_ParameterGroup))
		.SetDisplayName(LOCTEXT("ParameterGroupTabLabel", "ParameterGroup"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FUnLive2DAppStyle::GetStyleSetName(), "Persona.Tabs.AnimCurvePreviewer"));
}

void FUnLive2DAnimBaseViewEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(FUnLive2DMotionViewEditorTabs::ViewportID);
	InTabManager->UnregisterTabSpawner(FUnLive2DMotionViewEditorTabs::DetailsID);
	InTabManager->UnregisterTabSpawner(FUnLive2DMotionViewEditorTabs::AssetBrowserTab);
	InTabManager->UnregisterTabSpawner(FUnLive2DMotionViewEditorTabs::ParmeterGroupID);
}

void FUnLive2DAnimBaseViewEditor::PostUndo(bool bSuccess)
{
	OnPostUndo.Broadcast();
}

void FUnLive2DAnimBaseViewEditor::PostRedo(bool bSuccess)
{
	OnPostUndo.Broadcast();
}

FName FUnLive2DAnimBaseViewEditor::GetToolkitFName() const
{
	return FName("UnLive2DMotionViewEditor");
}

FText FUnLive2DAnimBaseViewEditor::GetBaseToolkitName() const
{
	return LOCTEXT("UnLive2DMotionEditorAppLabel", "UnLive2D Motion Editor");
}

FText FUnLive2DAnimBaseViewEditor::GetToolkitName() const
{
	return FText::FromString(UnLive2DAnimBeingEdited->GetName());
}

FText FUnLive2DAnimBaseViewEditor::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(UnLive2DAnimBeingEdited);
}

FLinearColor FUnLive2DAnimBaseViewEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

FString FUnLive2DAnimBaseViewEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("UnLive2DMotionAssetEditor");
}

FString FUnLive2DAnimBaseViewEditor::GetDocumentationLink() const
{
	return TEXT("Engine/UnLive2DAsset/UnLive2DMotionEditor");
}

void FUnLive2DAnimBaseViewEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(UnLive2DAnimBeingEdited);
}

UUnLive2D* FUnLive2DAnimBaseViewEditor::GetUnLive2DBeingEdited() const
{
	if (UnLive2DAnimBeingEdited == nullptr) return nullptr;

	return UnLive2DAnimBeingEdited->UnLive2D;
}

void FUnLive2DAnimBaseViewEditor::SetUnLive2DAnimBeingEdited(UUnLive2DAnimBase* NewAnimBase)
{
	if ((NewAnimBase != UnLive2DAnimBeingEdited) && (NewAnimBase != nullptr))
	{
		UUnLive2DAnimBase* OldAnimBase = UnLive2DAnimBeingEdited;
		UnLive2DAnimBeingEdited = NewAnimBase;

		RemoveEditingObject(OldAnimBase);
		AddEditingObject(NewAnimBase);

		UnLive2DAnimAssetListPtr->SelectAsset(NewAnimBase);

		OpenNewAnimBaseDocumentTab(NewAnimBase);

		UnLive2DParameterGroupPtr->UpDataUnLive2DAnimBase(NewAnimBase);
	}
}

TWeakObjectPtr<UUnLive2DRendererComponent> FUnLive2DAnimBaseViewEditor::GetUnLive2DRenderComponent() const
{
	if (!ViewportPtr.IsValid()) return nullptr;

	TSharedPtr<FUnLive2DViewportClient> UnLive2DViewportClient = StaticCastSharedPtr<FUnLive2DViewportClient>(ViewportPtr->GetViewportClient());
	if (!UnLive2DViewportClient.IsValid()) return nullptr;

	return UnLive2DViewportClient->GetUnLive2DRenderComponent();
}

EUnLive2DParameterAssetType::Type FUnLive2DAnimBaseViewEditor::GetUnLive2DParameterAssetType() const
{
	return UnLive2DAnimBeingEdited->IsA<UUnLive2DMotion>() ? EUnLive2DParameterAssetType::UnLive2DMotion : EUnLive2DParameterAssetType::UnLive2DExpression;
}

bool FUnLive2DAnimBaseViewEditor::GetUnLive2DParameterHasSaveData() const
{
	return !UnLive2DAnimBeingEdited->IsA<UUnLive2DMotion>();
}

bool FUnLive2DAnimBaseViewEditor::GetUnLive2DParameterAddParameterData() const
{
	return !UnLive2DAnimBeingEdited->IsA<UUnLive2DMotion>();
}

void FUnLive2DAnimBaseViewEditor::BindCommands()
{

}

void FUnLive2DAnimBaseViewEditor::ExtendToolbar()
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
		TSharedRef<IUnLive2DAssetFamily> AssetFamily = MangerModule.CreatePersonaAssetFamily(UnLive2DAnimBeingEdited);
		AddToolbarWidget(MangerModule.CreateAssetFamilyShortcutWidget(SharedThis(this), AssetFamily));
	}));
}

void FUnLive2DAnimBaseViewEditor::UpDataAnimBase()
{
	if (UnLive2DAnimBeingEdited == nullptr || UnLive2DAnimBeingEdited->UnLive2D == nullptr || !GetUnLive2DRenderComponent().IsValid()) return;

	if (UUnLive2DMotion* Motion = Cast<UUnLive2DMotion>(UnLive2DAnimBeingEdited))
	{
		//UnLive2DAnimBeingEdited->UnLive2D->PlayMotion(Motion);
		GetUnLive2DRenderComponent()->PlayMotion(Motion);
	}
	else if (UUnLive2DExpression* Expression = Cast<UUnLive2DExpression>(UnLive2DAnimBeingEdited))
	{
		//UnLive2DAnimBeingEdited->UnLive2D->PlayExpression(Expression);
		GetUnLive2DRenderComponent()->PlayExpression(Expression);
	}
}

TSharedRef<SDockTab> FUnLive2DAnimBaseViewEditor::SpawnTab_Viewport(const FSpawnTabArgs& Args)
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

TSharedRef<SDockTab> FUnLive2DAnimBaseViewEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		//.Icon(FUnLive2DStyle::GetBrush("LevelEditor.Tabs.Details"))
		.Label(LOCTEXT("DetailsTab_Title", "Details"))
		[
			SNew(SUnLive2DMotionPropertiesTabBody, SharedThis(this))
		];
}

TSharedRef<SDockTab> FUnLive2DAnimBaseViewEditor::SpawnTab_AssetBrowser(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		//.Icon(FUnLive2DStyle::GetBrush("LevelEditor.Tabs.ContentBrowser"))
		.Label(LOCTEXT("AssetBrowserTab", "AssetBrowser"))
		[
			//SNew(SUnLive2DMotionPropertiesTabBody, SharedThis(this))
			UnLive2DAnimAssetListPtr.ToSharedRef()
		];
}

TSharedRef<SDockTab> FUnLive2DAnimBaseViewEditor::SpawnTab_ParameterGroup(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("ParameterGroupTab_Title", "_ParameterGroup"))
		[
			SAssignNew(UnLive2DParameterGroupPtr, SUnLive2DParameterGroup, SharedThis(this), GetUnLive2DAnimBaseEdited())
		];

}

TSharedPtr<SDockTab> FUnLive2DAnimBaseViewEditor::OpenNewAnimBaseDocumentTab(UUnLive2DAnimBase* InAnimBase)
{
	TSharedPtr<SDockTab> OpenedTab;

	if (InAnimBase != nullptr)
	{
		FUnLive2DManagerModule& MangerModule = FModuleManager::GetModuleChecked<FUnLive2DManagerModule>("UnLive2DManager");

		TSharedRef<IUnLive2DAssetFamily> AssetFamily = MangerModule.CreatePersonaAssetFamily(InAnimBase);
		AssetFamily->RecordAssetOpened(FAssetData(InAnimBase));

		UpDataAnimBase();
	}

	return OpenedTab;
}

#undef LOCTEXT_NAMESPACE