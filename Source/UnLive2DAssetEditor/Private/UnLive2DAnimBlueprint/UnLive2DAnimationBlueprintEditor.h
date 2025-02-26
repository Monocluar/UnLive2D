
#pragma once

#include "CoreMinimal.h"
#include "Stats/Stats.h"
#include "Widgets/SWidget.h"
#include "IUnLive2DParameterEditorAsset.h"

class UUnLive2DRendererComponent;
class FDocumentTracker;

struct FUnLive2DAnimBlueprintEditorModes
{
	// Mode constants
	static const FName AnimationBlueprintEditorMode;
	static const FName AnimationBlueprintInterfaceEditorMode;

	static FText GetLocalizedMode(const FName InMode)
	{
		static TMap< FName, FText > LocModes;

		if (LocModes.Num() == 0)
		{
			LocModes.Add(AnimationBlueprintEditorMode, NSLOCTEXT("AnimationBlueprintEditorModes", "AnimationBlueprintEditorMode", "Animation Blueprint"));
			LocModes.Add(AnimationBlueprintInterfaceEditorMode, NSLOCTEXT("AnimationBlueprintEditorModes", "AnimationBlueprintInterface EditorMode", "Animation Blueprint Interface"));
		}

		check(InMode != NAME_None);
		const FText* OutDesc = LocModes.Find(InMode);
		check(OutDesc);
		return *OutDesc;
	}

private:
	FUnLive2DAnimBlueprintEditorModes() {}
};

namespace UnLive2DAnimationBlueprintEditorTabs
{
	extern const FName ViewportTab;
	extern const FName AssetBrowserTab;
	extern const FName CurveNamesTab;
	extern const FName GraphDocumentTab;
	extern const FName PropertiesTab;
};

class FUnLive2DAnimationBlueprintEditor : public IUnLive2DParameterEditorAsset, public FNotifyHook/*, public FEditorUndoClient*/
{
protected:

	typedef IUnLive2DParameterEditorAsset Super;

public:

	void InitUnLive2DAnimationBlueprintEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, class UUnLive2DAnimBlueprint* InAnimBlueprint);


public:
	FUnLive2DAnimationBlueprintEditor();

	virtual ~FUnLive2DAnimationBlueprintEditor();


protected:
	/** Undo Action**/
	void UndoAction();

	/** Redo Action **/
	void RedoAction();

protected:
	// IToolkit Interface
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;


	FString GetReferencerName() const override;
protected:
	virtual UUnLive2D* GetUnLive2DBeingEdited() const override;
	virtual TWeakObjectPtr<UUnLive2DRendererComponent> GetUnLive2DRenderComponent() const override;

	virtual EUnLive2DParameterAssetType::Type GetUnLive2DParameterAssetType() const override;

protected:
	//~ Begin FEditorUndoClient Interface
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;
	// End of FEditorUndoClient

	/** 调用postcompile来复制节点数据*/
	void OnPostCompile();


protected:
	/** FNotifyHook interface */
	virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged) override;

	// IToolkit interface
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	// End of IToolkit interface

public:
	// FSerializableObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	// End of FSerializableObject interface
public:

	virtual class UUnLive2DAnimBlueprint* GetBlueprintObj() const;
	// 将当前选定的节点指定给属性控件
	virtual void SetSelection(TArray<UObject*> SelectedObjects);
	// 获取当前选定的节点数
	virtual int32 GetNumberOfSelectedNodes() const;
	// 获取当前选定的节点集
	virtual FGraphPanelSelectionSet GetSelectedNodes() const;
	/** 检查当前是否可以粘贴节点 */
	virtual bool CanPasteNodes() const;
	/** 在特定位置粘贴节点 */
	virtual void PasteNodesHere(const FVector2D& Location);
	/**
	 * 获取当前选定节点的边界区域
	 * @param Rect 最终输出边界区域，包括填充
	 * @param Padding 要添加到边界所有边的填充量
	 */
	virtual bool GetBoundsForSelectedNodes(class FSlateRect& Rect, float Padding);

private:

	void BindCommands();


private:

	TSharedRef<SDockTab> SpawnTab_Viewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_AssetBrowser(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_GraphDocument(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Properties(const FSpawnTabArgs& Args);

	/** Create new graph editor widget */
	TSharedRef<SGraphEditor> CreateGraphEditorWidget();

	// 创建选项卡指向的所有内部小部件
	void CreateInternalWidgets();

	/** Extend menu */
	void ExtendMenu();

	/** Extend toolbar */
	void ExtendToolbar();

private:

	/** 将内容浏览器同步到当前选定的节点 */
	void SyncInBrowser();
	/** 是否可以将内容浏览器同步到当前选择的节点 */
	bool CanSyncInBrowser() const;

	// 删除一个输入节点
	void DeleteInput();
	// 是否可以删除选中的输入节点
	bool CanDeleteInput() const { return true; };

	/** 删除图表中选中的节点 */
	void DeleteSelectedNodes();
	/** 是否可以删除选中的节点 */
	bool CanDeleteNodes() const;
	// 仅删除当前选定的可以复制的节点
	void DeleteSelectedDuplicatableNodes();

	// 选中所有节点
	void SelectAllNodes();
	// 是否可以选中所有节点
	bool CanSelectAllNodes() const { return true; };

	// 复制选中的节点
	void CopySelectedNodes();
	// 是否可以复制节点
	bool CanCopyNodes() const;

	// 剪切选中的节点
	void CutSelectedNodes();
	// 是否可以剪切选中的节点
	bool CanCutNodes() const;

	// 粘贴复制的节点数据
	void PasteNodes();

	// 复制一样节点数据
	void DuplicateNodes();
	// 是否复制一样节点数据
	bool CanDuplicateNodes() const;

private:
	// 播放动画蓝图
	void PlayUnLive2DAnimBlueprint();
	// 播放节点动画
	void PlayAnimNode();
	// 是否可以播放当前选择的节点
	bool CanPlayAnimNode() const;
	// 停止播放
	void Stop();

protected:

	/** Called when the selection changes in the GraphEditor */
	void OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection);
	
	/**
	 * Called when a node's title is committed for a rename
	 *
	 * @param	NewText				New title text
	 * @param	CommitInfo			How text was committed
	 * @param	NodeBeingChanged	The node being changed
	 */
	void OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged);

	/** 播放单个指定节点 */
	void PlaySingleNode(UEdGraphNode* Node);

private:

	void AssetBrowser_OnMotionDoubleClicked(const FAssetData& AssetData) const;

	bool AssetBrowser_FilterMotionBasedOnParentClass(const FAssetData& AssetData) const;

public:
	/** Delegate called after an undo operation for child widgets to refresh */
	FSimpleMulticastDelegate OnPostUndo;

private:

	TSharedPtr<FDocumentTracker> DocumentManager;

	/** New Graph Editor */
	TSharedPtr<SGraphEditor> UnLive2DAnimBlueprintGraphEditor;

	TSharedPtr<class IUnLive2DToolkit> UnLive2DManagerToolkit;

	/** The extender to pass to the level editor to extend it's window menu */
	TSharedPtr<FExtender> MenuExtender;

	/** Toolbar extender */
	TSharedPtr<FExtender> ToolbarExtender;

#if ENGINE_MAJOR_VERSION < 5
	UUnLive2DRendererComponent* DebuggedUnLive2DComponent;

	UUnLive2DAnimBlueprint* UnLive2DAnimBlueprintEdited;
#else
	TObjectPtr<UUnLive2DRendererComponent> DebuggedUnLive2DComponent;

	TObjectPtr<UUnLive2DAnimBlueprint> UnLive2DAnimBlueprintEdited;
#endif

	/** Command list for this editor */
	TSharedPtr<FUICommandList> GraphEditorCommands;

	/** Properties tab */
	TSharedPtr<class IDetailsView> UnLive2DAnimProperties;

	TSharedPtr<class SUnLive2DAnimBlueprintEditorViewport> ViewportPtr;
};