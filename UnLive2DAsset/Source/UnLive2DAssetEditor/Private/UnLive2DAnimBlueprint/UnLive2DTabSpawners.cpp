#include "UnLive2DTabSpawners.h"
#include "UnLive2DAnimBlueprint/UnLive2DAnimationBlueprintEditor.h"
#include "Framework/Docking/WorkspaceItem.h"
#include "SUnLive2DAnimBlueprintEditorViewport.h"
#include "ContentBrowserModule.h"
#include "UnLive2DMotion.h"
#include "IContentBrowserSingleton.h"
#include "Animation/UnLive2DAnimBlueprint.h"

#define LOCTEXT_NAMESPACE "UnLive2DTabSpawners"

FUnLive2DAnimViewportSummoner::FUnLive2DAnimViewportSummoner(TSharedPtr<class FAssetEditorToolkit> InHostingApp, const FUnLive2DAssetViewportArgs& InArgs)
	: Super(UnLive2DAnimationBlueprintEditorTabs::ViewportTab, InHostingApp)
	, BlueprintEditor(InArgs.BlueprintEditor)
	, OnViewportCreated(InArgs.OnUnLive2DViewportCreated)
	, Extenders(InArgs.Extenders)
	, ContextName(InArgs.ContextName)
{
	TabLabel = LOCTEXT("ViewportTabTitle", "Viewport");
	TabIcon = FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports");

	bIsSingleton = true;

	ViewMenuDescription = LOCTEXT("ViewportViewFormat", "Viewport");
	ViewMenuTooltip = NSLOCTEXT("PersonaModes", "ViewportView_ToolTip", "Shows the viewport");

	
}

TSharedRef<SWidget> FUnLive2DAnimViewportSummoner::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	TSharedRef<SUnLive2DAnimBlueprintEditorViewport> ViewportPtr = SNew(SUnLive2DAnimBlueprintEditorViewport)
		.UnLive2DAnimBlueprintEdited(this, &FUnLive2DAnimViewportSummoner::GetUnLive2DAnimBlueprint);

	OnViewportCreated.ExecuteIfBound(ViewportPtr);

	return ViewportPtr;
}

FTabSpawnerEntry& FUnLive2DAnimViewportSummoner::RegisterTabSpawner(TSharedRef<FTabManager> TabManager, const FApplicationMode* CurrentApplicationMode) const
{
	FTabSpawnerEntry& SpawnerEntry = Super::RegisterTabSpawner(TabManager, nullptr);

	if (CurrentApplicationMode)
	{
		// find an existing workspace item or create new
		TSharedPtr<FWorkspaceItem> GroupItem = nullptr;

		for (const TSharedRef<FWorkspaceItem>& Item : CurrentApplicationMode->GetWorkspaceMenuCategory()->GetChildItems())
		{
			if (Item->GetDisplayName().ToString() == LOCTEXT("ViewportsSubMenu", "Viewports").ToString())
			{
				GroupItem = Item;
				break;
			}
		}

		if (!GroupItem.IsValid())
		{
			GroupItem = CurrentApplicationMode->GetWorkspaceMenuCategory()->AddGroup(LOCTEXT("ViewportsSubMenu", "Viewports"), NSLOCTEXT("PersonaModes", "ViewportsSubMenu_Tooltip", "Open a new viewport on the scene"), FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));
		}

		SpawnerEntry.SetGroup(GroupItem.ToSharedRef());
	}

	return SpawnerEntry;

}

UUnLive2DAnimBlueprint* FUnLive2DAnimViewportSummoner::GetUnLive2DAnimBlueprint() const
{
	if (BlueprintEditor.IsValid() && BlueprintEditor.Pin())
	{
		return Cast<UUnLive2DAnimBlueprint>(BlueprintEditor.Pin()->GetBlueprintObj());
	}

	return nullptr;
}

FUnLive2DMotionAssetBrowserSummoner::FUnLive2DMotionAssetBrowserSummoner(TSharedPtr<class FAssetEditorToolkit> InHostingApp, TSharedPtr<class FBlueprintEditor> InBlueprintEditor)
	: Super(UnLive2DAnimationBlueprintEditorTabs::AssetBrowserTab, InHostingApp)
	, BlueprintEditor(InBlueprintEditor)
{
	TabLabel = NSLOCTEXT("PersonaModes", "AssetBrowserTabTitle", "Asset Browser");
	TabIcon = FSlateIcon(FEditorStyle::GetStyleSetName(), "ContentBrowser.TabIcon");

	bIsSingleton = true;

	ViewMenuDescription = NSLOCTEXT("PersonaModes", "AssetBrowser", "Asset Browser");
	ViewMenuTooltip = NSLOCTEXT("PersonaModes", "AssetBrowser_ToolTip", "Shows the animation asset browser");
}

TSharedRef<SWidget> FUnLive2DMotionAssetBrowserSummoner::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	FAssetPickerConfig AssetPickerConfig;
	AssetPickerConfig.Filter.ClassNames.Add(UUnLive2DMotion::StaticClass()->GetFName());
	AssetPickerConfig.OnAssetDoubleClicked = FOnAssetDoubleClicked::CreateSP(this, &FUnLive2DMotionAssetBrowserSummoner::OnMotionDoubleClicked);
	AssetPickerConfig.OnShouldFilterAsset = FOnShouldFilterAsset::CreateSP(this, &FUnLive2DMotionAssetBrowserSummoner::FilterMotionBasedOnParentClass);
	AssetPickerConfig.bAllowNullSelection = true;
	AssetPickerConfig.InitialAssetViewType = EAssetViewType::Column;

	//AssetPickerConfig.SyncToAssetsDelegates.Add(&SyncToAssetsDelegate);

	return ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig);
}

void FUnLive2DMotionAssetBrowserSummoner::OnMotionDoubleClicked(const FAssetData& AssetData) const
{
	if (UObject* RawAsset = AssetData.GetAsset())
	{
		if (UUnLive2DMotion* Motion = Cast<UUnLive2DMotion>(RawAsset))
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Motion);
		}
	}
}

bool FUnLive2DMotionAssetBrowserSummoner::FilterMotionBasedOnParentClass(const FAssetData& AssetData) const
{
	if (!AssetData.IsValid()) return true;

	UUnLive2DMotion* TargetUnLive2DMotion = Cast<UUnLive2DMotion>(AssetData.GetAsset());

	if (UUnLive2DAnimBlueprint* AnimBlueprintPtr = GetUnLive2DAnimBlueprint())
	{
		if (TargetUnLive2DMotion)
		{
			return !(TargetUnLive2DMotion->UnLive2D == AnimBlueprintPtr->TargetUnLive2D);
		}
	}

	return true;
}

UUnLive2DAnimBlueprint* FUnLive2DMotionAssetBrowserSummoner::GetUnLive2DAnimBlueprint() const
{
	if (BlueprintEditor.IsValid() && BlueprintEditor.Pin())
	{
		return Cast<UUnLive2DAnimBlueprint>(BlueprintEditor.Pin()->GetBlueprintObj());
	}

	return nullptr;
}

#undef LOCTEXT_NAMESPACE