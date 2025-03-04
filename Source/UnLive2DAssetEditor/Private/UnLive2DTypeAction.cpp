#include "UnLive2DTypeAction.h"
#include "UnLive2D.h"
#include "UnLive2DViewEditor/UnLive2DViewEditor.h"
#include "ToolMenuSection.h"
#include "ObjectEditorUtils.h"
#include "UnLive2DEditorStyle.h"
#include "UnLive2DAnimBlueprint/UnLive2DBlueprintFactory.h"
#include "ContentBrowserModule.h"
#include "Animation/UnLive2DAnimBlueprint.h"
#include "IContentBrowserSingleton.h"
#include "UnLive2DCubismCore.h"

#define LOCTEXT_NAMESPACE "FUnLive2DAssetEditorModule"

FUnLive2DTypeAction::FUnLive2DTypeAction(uint32 InAssetCategory)
	: FAssetTypeActions_Base()
	, AssetType(InAssetCategory)
{

}

uint32 FUnLive2DTypeAction::GetCategories()
{
	return AssetType;
}

FText FUnLive2DTypeAction::GetName() const
{
	return LOCTEXT("AssetTypeActions_UnLive2DDataAsset", "UnLive2D");
}

UClass* FUnLive2DTypeAction::GetSupportedClass() const
{
	return UUnLive2D::StaticClass();
}

FColor FUnLive2DTypeAction::GetTypeColor() const
{
	return FColor(10,10,126);
}

void FUnLive2DTypeAction::GetActions(const TArray<UObject*>& InObjects, struct FToolMenuSection& Section)
{
	TArray<TWeakObjectPtr<UUnLive2D>> UnLive2DArr = GetTypedWeakObjectPtrs<UUnLive2D>(InObjects);

	/*Section.AddSubMenu(
		"CreateUnLive2DSubmenu",
		NSLOCTEXT("AssetTypeActions", "CreateSkeletonSubmenu", "Create"),
		NSLOCTEXT("AssetTypeActions", "CreateSkeletonSubmenu_ToolTip", "Create assets for this skeleton"),
		FNewMenuDelegate::CreateSP(this, &FUnLive2DTypeAction::FillCreateMenu, UnLive2DArr),
		false,
		FSlateIcon(FUnLive2DAppStyle::GetStyleSetName(), "Persona.AssetActions.CreateAnimAsset")
	);*/
}

void FUnLive2DTypeAction::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor /*= TSharedPtr<IToolkitHost>()*/)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		if (UUnLive2D* UnLive2D = Cast<UUnLive2D>(*ObjIt))
		{
			TSharedRef<FUnLive2DViewEditor> NewUnLive2DEditor (new FUnLive2DViewEditor());

			NewUnLive2DEditor->InitUnLive2DViewEditor(Mode, EditWithinLevelEditor, UnLive2D);

		}
	}
}

void FUnLive2DTypeAction::FillCreateMenu(FMenuBuilder& MenuBuilder, TArray<TWeakObjectPtr<UUnLive2D>> UnLive2DArr)
{

	MenuBuilder.BeginSection("CreateAnimAssets", NSLOCTEXT("AssetTypeActions","CreateAnimAssetsMenuHeading", "Anim Assets"));
	{
		MenuBuilder.AddMenuEntry(
			NSLOCTEXT("AssetTypeActions", "Skeleton_NewAnimBlueprint", "Anim Blueprint"),
			NSLOCTEXT("AssetTypeActions", "Skeleton_NewAnimBlueprintTooltip", "Creates an Anim Blueprint using the selected skeleton."),
			FSlateIcon(FUnLive2DEditorStyle::GetStyleSetName(), "ClassIcon.UnLive2DAnimBlueprint"),
			FUIAction(
				FExecuteAction::CreateSP(this, &FUnLive2DTypeAction::CreateNewAnimBlueprint, UnLive2DArr),
				FCanExecuteAction()
			)
		);
	}
	MenuBuilder.EndSection();
}

void FUnLive2DTypeAction::CreateNewAnimBlueprint(TArray<TWeakObjectPtr<UUnLive2D>> UnLive2DArr)
{
	const FString DefaultSuffix = TEXT("_AnimBlueprint");

	if (UnLive2DArr.Num() == 1)
	{
		UUnLive2D* UnLive2D = Cast<UUnLive2D>(UnLive2DArr[0].Get());
		if (UnLive2D == nullptr) return;

		// Determine an appropriate name for inline-rename
		FString AssetName;
		FString PackageName;
		FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
		AssetToolsModule.Get().CreateUniqueAssetName(UnLive2D->GetOutermost()->GetName(), DefaultSuffix, PackageName, AssetName);

		UUnLive2DBlueprintFactory* Factory = NewObject<UUnLive2DBlueprintFactory>();
		Factory->TargetUnLive2D = UnLive2D;

		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		ContentBrowserModule.Get().CreateNewAsset(AssetName, FPackageName::GetLongPackagePath(PackageName), UUnLive2DAnimBlueprint::StaticClass(), Factory);
	}
	else
	{
		TArray<UObject*> AssetsToSync;
		for (auto ObjIt = UnLive2DArr.CreateConstIterator(); ObjIt; ++ObjIt)
		{
			UUnLive2D* UnLive2D = Cast<UUnLive2D>(ObjIt->Get());
			if (UnLive2D == nullptr) continue;

			// Determine an appropriate name for inline-rename
			FString AssetName;
			FString PackageName;
			FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
			AssetToolsModule.Get().CreateUniqueAssetName(UnLive2D->GetOutermost()->GetName(), DefaultSuffix, PackageName, AssetName);

			// Create the anim blueprint factory used to generate the asset
			UUnLive2DBlueprintFactory* Factory = NewObject<UUnLive2DBlueprintFactory>();
			Factory->TargetUnLive2D = UnLive2D;

			UObject* NewAsset = AssetToolsModule.Get().CreateAsset(AssetName, FPackageName::GetLongPackagePath(PackageName), UUnLive2DAnimBlueprint::StaticClass(), Factory);

			if (NewAsset)
			{
				AssetsToSync.Add(NewAsset);
			}
		}

		if (AssetsToSync.Num() > 1)
		{
			FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
			ContentBrowserModule.Get().SyncBrowserToAssets(AssetsToSync, /*bAllowLockedBrowsers=*/true);

		}
	}
}

#undef LOCTEXT_NAMESPACE