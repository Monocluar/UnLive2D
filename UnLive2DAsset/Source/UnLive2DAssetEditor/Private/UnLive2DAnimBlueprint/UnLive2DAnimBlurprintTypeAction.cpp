#include "UnLive2DAnimBlurprintTypeAction.h"
#include "Animation/UnLive2DAnimBlueprint.h"
#include "UnLive2DBlueprintFactory.h"
#include "Widgets/SCompoundWidget.h"
#include "EditorAnimUtils.h"
#include "UnLive2DAnimationBlueprintEditor.h"
#include "UnLive2DEditorStyle.h"
#include "SBlueprintDiff.h"
#include "ThumbnailRendering/SceneThumbnailInfo.h"
#include "ContentBrowserModule.h"
#include "Styling/SlateIconFinder.h"
#include "ToolMenuSection.h"
#include "UnLive2D.h"
#include "IContentBrowserSingleton.h"

#define LOCTEXT_NAMESPACE "FUnLive2DAssetEditorModule"

FUnLive2DAnimBlurprintTypeAction::FUnLive2DAnimBlurprintTypeAction(uint32 InAssetCategory)
	: FAssetTypeActions_Base()
	, AssetType(InAssetCategory)
{

}

uint32 FUnLive2DAnimBlurprintTypeAction::GetCategories()
{
	return AssetType;
}

TSharedPtr<SWidget> FUnLive2DAnimBlurprintTypeAction::GetThumbnailOverlay(const FAssetData& AssetData) const
{
	const FSlateBrush* Icon = FSlateIconFinder::FindIconBrushForClass(UUnLive2DAnimBlueprint::StaticClass());

	return SNew(SBorder)
		.BorderImage(FAppStyle::GetNoBrush())
		.Visibility(EVisibility::HitTestInvisible)
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 3.0f))
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		[
			SNew(SImage)
			.Image(Icon)
		];
}

FText FUnLive2DAnimBlurprintTypeAction::GetName() const
{
	return LOCTEXT("AssetTypeActions_UnLive2DAnimBlurprintTypeAction", "UnLive2D AnimBlurprint");
}

FColor FUnLive2DAnimBlurprintTypeAction::GetTypeColor() const
{
	return FColor(10,10,126);
}

UClass* FUnLive2DAnimBlurprintTypeAction::GetSupportedClass() const
{
	return UUnLive2DAnimBlueprint::StaticClass();
}

void FUnLive2DAnimBlurprintTypeAction::GetActions(const TArray<UObject*>& InObjects, struct FToolMenuSection& Section)
{
	FAssetTypeActions_Base::GetActions(InObjects, Section);

	TArray<TWeakObjectPtr<UUnLive2DAnimBlueprint>> AnimBlueprints = GetTypedWeakObjectPtrs<UUnLive2DAnimBlueprint>(InObjects);

	Section.AddMenuEntry(
		"AnimBlueprint_FindUnLive2D",
		LOCTEXT("AnimBlueprint_FindUnLive2D", "Find UnLive2D"),
		LOCTEXT("AnimBlueprint_FindFindUnLive2DTooltip", "在内容浏览器中寻找UnLive2D资源数据"),
		FSlateIcon(FUnLive2DEditorStyle::GetStyleSetName(), "ClassIcon.UnLive2D"),
		FUIAction(
			FExecuteAction::CreateSP(this, &FUnLive2DAnimBlurprintTypeAction::ExecuteFindUnLive2D, AnimBlueprints),
			FCanExecuteAction()
		)
	);

}

void FUnLive2DAnimBlurprintTypeAction::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor /*= TSharedPtr<IToolkitHost>() */)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;
	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		UUnLive2DAnimBlueprint* AnimBlueprint = Cast<UUnLive2DAnimBlueprint>(*ObjIt);
		if (AnimBlueprint != NULL)
		{
			if (!AnimBlueprint->TargetUnLive2D)
			{
				FText ShouldRetargetMessage = LOCTEXT("ShouldRetarget_Message", "找不到动画蓝图的UnLive2D数据 '{BlueprintName}' 你想选一个新的吗?");
				FFormatNamedArguments Arguments;
				Arguments.Add(TEXT("BlueprintName"), FText::FromString(AnimBlueprint->GetName()));

				if (FMessageDialog::Open(EAppMsgType::YesNo, FText::Format(ShouldRetargetMessage, Arguments)) == EAppReturnType::Yes)
				{
					bool bDuplicateAssets = false;
					TArray<UObject*> AnimBlueprints;
					AnimBlueprints.Add(AnimBlueprint);
					RetargetAssets(AnimBlueprints, bDuplicateAssets);
				}
			}
			else
			{
				const bool bBringToFrontIfOpen = true;
#if WITH_EDITOR
				if (IAssetEditorInstance* EditorInstance = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(AnimBlueprint, bBringToFrontIfOpen))
				{
					EditorInstance->FocusWindow(AnimBlueprint);
				}
				else
#endif
				{
					TSharedPtr< FUnLive2DAnimationBlueprintEditor > NewAnimationBlueprintEditor = MakeShared<FUnLive2DAnimationBlueprintEditor>();
					NewAnimationBlueprintEditor->InitUnLive2DAnimationBlueprintEditor(Mode, EditWithinLevelEditor, AnimBlueprint);
				}
			}
		}
		else
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FailedToLoadCorruptAnimBlueprint", "无法加载动画蓝图，因为它已损坏."));
		}
	}
}

void FUnLive2DAnimBlurprintTypeAction::ExecuteFindUnLive2D(TArray<TWeakObjectPtr<UUnLive2DAnimBlueprint>> Objects)
{
	TArray<UObject*> ObjectsToSync;
	for (auto ObjIt = Objects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto Object = (*ObjIt).Get();
		if (Object)
		{
			UUnLive2D* UnLive2D = Object->TargetUnLive2D;
			if (UnLive2D)
			{
				ObjectsToSync.AddUnique(UnLive2D);
			}
		}
	}

	if (ObjectsToSync.Num() > 0)
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		ContentBrowserModule.Get().SyncBrowserToAssets(ObjectsToSync, /*bAllowLockedBrowsers=*/true);
	}
}

void FUnLive2DAnimBlurprintTypeAction::RetargetAssets(TArray<UObject*> InAnimBlueprints, bool bDuplicateAssets)
{
	bool bRemapReferencedAssets = false;
	UUnLive2D* OldUnLive2D = NULL;

	if (InAnimBlueprints.Num() > 0)
	{
		UUnLive2DAnimBlueprint* AnimBP = CastChecked<UUnLive2DAnimBlueprint>(InAnimBlueprints[0]);
		OldUnLive2D = AnimBP->TargetUnLive2D;
	}

	const FText Message = LOCTEXT("RemapSkeleton_Warning", "选择要将此资源重新映射到的骨架.");
	auto AnimBlueprints = GetTypedWeakObjectPtrs<UObject>(InAnimBlueprints);

	/*SUnLive2DAnimationRemapSkeleton::ShowWindow(OldUnLive2D, Message, bDuplicateAssets, FOnRetargetAnimation::CreateSP(this, &FUnLive2DAnimBlurprintTypeAction::RetargetAnimationHandler, AnimBlueprints));*/

}

#undef LOCTEXT_NAMESPACE