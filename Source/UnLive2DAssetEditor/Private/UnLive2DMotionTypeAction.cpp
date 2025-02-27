#include "UnLive2DMotionTypeAction.h"
#include "Animation/UnLive2DMotion.h"
#include "UnLive2DMotionEditor/UnLive2DMotionViewEditor.h"

#define LOCTEXT_NAMESPACE "FUnLive2DMotionTypeAction"

FUnLive2DMotionTypeAction::FUnLive2DMotionTypeAction(uint32 InAssetCategory)
	: FAssetTypeActions_Base()
	, AssetType(InAssetCategory)
{

}

uint32 FUnLive2DMotionTypeAction::GetCategories()
{
	return AssetType;
}

FText FUnLive2DMotionTypeAction::GetName() const
{
	return LOCTEXT("AssetTypeActions_UnLive2DMotionDataAsset", "UnLive2DMotion");
}

UClass* FUnLive2DMotionTypeAction::GetSupportedClass() const
{
	return UUnLive2DMotion::StaticClass();
}

FColor FUnLive2DMotionTypeAction::GetTypeColor() const
{
	return FColor(10,10,126);
}

void FUnLive2DMotionTypeAction::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor /*= TSharedPtr<IToolkitHost>()*/)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		if (UUnLive2DMotion* UnLive2DMotion = Cast<UUnLive2DMotion>(*ObjIt))
		{
			TSharedPtr<FUnLive2DAnimBaseViewEditor> NewUnLive2DEditor = MakeShared<FUnLive2DAnimBaseViewEditor>();

			NewUnLive2DEditor->InitUnLive2DAnimViewEditor(Mode, EditWithinLevelEditor, UnLive2DMotion);
		}
	}
}

#undef LOCTEXT_NAMESPACE