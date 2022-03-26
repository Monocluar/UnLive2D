#pragma once

#include "CoreMinimal.h"
#include "WorkflowOrientedApp/WorkflowTabFactory.h"
#include "ContentBrowserDelegates.h"
#include "WorkflowOrientedApp/ApplicationMode.h"

DECLARE_DELEGATE_OneParam(FOnUnLive2DViewportCreated, const TSharedRef<class SCompoundWidget>&);
DECLARE_DELEGATE_OneParam(FOnUnLive2DOpenNewAsset, UObject* /* InAsset */);

struct FUnLive2DAssetViewportArgs
{
	FUnLive2DAssetViewportArgs()
	{}

public:
	/** 可选的蓝图编辑器，我们可以嵌入其中 */
	TSharedPtr<class FBlueprintEditor> BlueprintEditor;

	/** 创建视口时激发的代理 */
	FOnUnLive2DViewportCreated OnUnLive2DViewportCreated;

	/** Menu extenders */
	TArray<TSharedPtr<FExtender>> Extenders;

	/** The context in which we are constructed. Used to persist various settings. */
	FName ContextName;
};


struct FUnLive2DAnimViewportSummoner : public FWorkflowTabFactory
{
	typedef FWorkflowTabFactory Super;

public:
	FUnLive2DAnimViewportSummoner(TSharedPtr<class FAssetEditorToolkit> InHostingApp, const FUnLive2DAssetViewportArgs& InArgs);


protected:

	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;


	virtual FTabSpawnerEntry& RegisterTabSpawner(TSharedRef<FTabManager> TabManager, const FApplicationMode* CurrentApplicationMode) const override;

private:

	UUnLive2DAnimBlueprint* GetUnLive2DAnimBlueprint() const;

private:
	TWeakPtr<FBlueprintEditor> BlueprintEditor;
	FOnUnLive2DViewportCreated OnViewportCreated;
	TArray<TSharedPtr<FExtender>> Extenders;
	FName ContextName;
};

struct FUnLive2DMotionAssetBrowserSummoner : public FWorkflowTabFactory
{
	typedef FWorkflowTabFactory Super;

public:
	FUnLive2DMotionAssetBrowserSummoner(TSharedPtr<class FAssetEditorToolkit> InHostingApp, TSharedPtr<class FBlueprintEditor> InBlueprintEditor);

protected:

	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;

private:
	void OnMotionDoubleClicked(const FAssetData& AssetData) const;
	bool FilterMotionBasedOnParentClass(const FAssetData& AssetData) const;

	UUnLive2DAnimBlueprint* GetUnLive2DAnimBlueprint() const;

private:
	TWeakPtr<FBlueprintEditor> BlueprintEditor;

	// 委托将资产选择器同步到所选资产
	FSyncToAssetsDelegate SyncToAssetsDelegate;
};