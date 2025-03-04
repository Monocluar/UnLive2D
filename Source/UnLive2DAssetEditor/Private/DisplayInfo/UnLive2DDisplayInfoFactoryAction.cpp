#include "DisplayInfo/UnLive2DDisplayInfoFactoryAction.h"
#include "Editor/EditorUnLive2DDisplayInfo.h"

FUnLive2DDisplayInfoFactoryAction::FUnLive2DDisplayInfoFactoryAction(uint32 InAssetCategory)
	: FAssetTypeActions_Base()
	, AssetType(InAssetCategory)
{

}

uint32 FUnLive2DDisplayInfoFactoryAction::GetCategories()
{
	return AssetType;
}

FText FUnLive2DDisplayInfoFactoryAction::GetName() const
{
	return NSLOCTEXT("UnLive2DAssetEditor", "AssetTypeActions_UnLive2DDisplayInfoDataAsset", "EditorUnLive2DDisplayInfo");

}

UClass* FUnLive2DDisplayInfoFactoryAction::GetSupportedClass() const
{
	return UEditorUnLive2DDisplayInfo::StaticClass();
}

FColor FUnLive2DDisplayInfoFactoryAction::GetTypeColor() const
{
	return FColor(0, 169, 255);
}