#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "ContentBrowserDelegates.h"

class FUnLive2DMotionViewEditor;
class UObject;
class UUnLive2DMotion;

class SUnLive2DMotionAssetBrowser : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUnLive2DMotionAssetBrowser)
	{}

	SLATE_END_ARGS()

public:

	void Construct(const FArguments& InArgs, TSharedPtr<FUnLive2DMotionViewEditor> InUnLive2DMotionEditor);

	// SWidget interface
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	// End of SWidget interface

	void SelectAsset(UObject* InAsset);

protected:
	void RebuildWidget(UUnLive2DMotion* UnLive2DMotion);

private:

	void OnMotionSelected(const FAssetData& AssetData);
	void OnMotionDoubleClicked(const FAssetData& AssetData);
	bool FilterMotionBasedOnParentClass(const FAssetData& AssetData);

private:

	/** <选择的UnLive2D Motion资源 */
	FAssetData TargetUnLive2DMotionAsset;

	TWeakObjectPtr<UUnLive2DMotion> SourceMotionPtr;

	TWeakPtr<FUnLive2DMotionViewEditor> UnLive2DMotionEditor;

	// 委托将资产选择器同步到所选资产
	FSyncToAssetsDelegate SyncToAssetsDelegate;
};