#include "UnLive2DMotionTypeAction.h"

#define LOCTEXT_NAMESPACE "FUnLive2DMotionTypeAction"

FUnLive2DMotionTypeAction::FUnLive2DMotionTypeAction(uint32 InAssetCategory)
	:AssetType(InAssetCategory)
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

#undef LOCTEXT_NAMESPACE