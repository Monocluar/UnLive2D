#include "UnLive2DTypeAction.h"
#include "UnLive2D.h"
#include "UnLive2DViewEditor/UnLive2DViewEditor.h"

#define LOCTEXT_NAMESPACE "FUnLive2DAssetEditorModule"

FUnLive2DTypeAction::FUnLive2DTypeAction(uint32 InAssetCategory)
	:AssetType(InAssetCategory)
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

#undef LOCTEXT_NAMESPACE