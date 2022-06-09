#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "ContentBrowserDelegates.h"

class FUnLive2DAnimBaseViewEditor;
class UObject;
class UUnLive2DAnimBase;

class SUnLive2DAnimBaseAssetBrowser : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUnLive2DAnimBaseAssetBrowser)
	{}

	SLATE_END_ARGS()

public:

	void Construct(const FArguments& InArgs, TSharedPtr<FUnLive2DAnimBaseViewEditor> InUnLive2DAnimBaseEditor);

	// SWidget interface
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	// End of SWidget interface

	void SelectAsset(UObject* InAsset);

protected:
	void RebuildWidget(UUnLive2DAnimBase* UnLive2DAnimBas);

private:

	void OnAnimBaseSelected(const FAssetData& AssetData);
	void OnAnimBaseDoubleClicked(const FAssetData& AssetData);
	bool FilterAnimBaseBasedOnParentClass(const FAssetData& AssetData);

private:

	/** <选择的UnLive2D AnimBase资源 */
	FAssetData TargetUnLive2DAnimBaseAsset;

	TWeakObjectPtr<UUnLive2DAnimBase> SourceAnimBasePtr;

	TWeakPtr<FUnLive2DAnimBaseViewEditor> UnLive2DAnimBaseEditor;

	// 委托将资产选择器同步到所选资产
	FSyncToAssetsDelegate SyncToAssetsDelegate;
};