
#pragma once

#include "CoreMinimal.h"
#include "Stats/Stats.h"
#include "Widgets/SWidget.h"
#include "EditorUndoClient.h"
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"

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
	extern const FName DetailsTab;
	extern const FName ViewportTab;
	extern const FName AssetBrowserTab;
	extern const FName CurveNamesTab;
	extern const FName GraphDocumentTab;
};

class FUnLive2DAnimationBlueprintEditor : public FWorkflowCentricApplication ,  public FEditorUndoClient , public FGCObject /*public FNotifyHook, public FEditorUndoClient*/
{
protected:

	typedef FWorkflowCentricApplication Super;

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
	

protected:
	// IToolkit Interface
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;

protected:
	//~ Begin FEditorUndoClient Interface
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;
	// End of FEditorUndoClient

	/** 调用postcompile来复制节点数据*/
	void OnPostCompile();


protected:

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

private:

	void BindCommands();

public:

	/** Handle the viewport being created */
	void HandleViewportCreated(const TSharedRef<class SCompoundWidget>& InPersonaViewport);

private:

	TSharedRef<SDockTab> SpawnTab_Viewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_AssetBrowser(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_GraphDocument(const FSpawnTabArgs& Args);

	/** Create new graph editor widget */
	TSharedRef<SGraphEditor> CreateGraphEditorWidget();

	/** Extend menu */
	void ExtendMenu();

	/** Extend toolbar */
	void ExtendToolbar();

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

	/** Plays a single specified node */
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

	UUnLive2DRendererComponent* DebuggedUnLive2DComponent;

	UUnLive2DAnimBlueprint* UnLive2DAnimBlueprintEdited;

	/** Command list for this editor */
	TSharedPtr<FUICommandList> GraphEditorCommands;
};