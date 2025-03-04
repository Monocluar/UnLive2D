#include "UnLive2DExpressionAction.h"
#include "Animation/UnLive2DExpression.h"
#include "UnLive2DMotionEditor/UnLive2DMotionViewEditor.h"

#define LOCTEXT_NAMESPACE "UnLive2DAssetEditor"

FUnLive2DExpressionAction::FUnLive2DExpressionAction(uint32 InAssetCategory)
	: FAssetTypeActions_Base()
	, AssetType(InAssetCategory)
{
}

uint32 FUnLive2DExpressionAction::GetCategories()
{
	return AssetType;
}


FText FUnLive2DExpressionAction::GetName() const
{
	return LOCTEXT("AssetTypeActions_UnLive2DExpressionDataAsset", "UnLive2DExpression");
}

UClass* FUnLive2DExpressionAction::GetSupportedClass() const
{
	return UUnLive2DExpression::StaticClass();
}

FColor FUnLive2DExpressionAction::GetTypeColor() const
{
	return FColor(0.f, 126, 126);
}

void FUnLive2DExpressionAction::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor /*= TSharedPtr<IToolkitHost>()*/)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		if (UUnLive2DExpression* UnLive2DExpression = Cast<UUnLive2DExpression>(*ObjIt))
		{
			TSharedPtr<FUnLive2DAnimBaseViewEditor> NewUnLive2DEditor = MakeShared<FUnLive2DAnimBaseViewEditor>();

			NewUnLive2DEditor->InitUnLive2DAnimViewEditor(Mode, EditWithinLevelEditor, UnLive2DExpression);
		}
	}
}

#undef LOCTEXT_NAMESPACE