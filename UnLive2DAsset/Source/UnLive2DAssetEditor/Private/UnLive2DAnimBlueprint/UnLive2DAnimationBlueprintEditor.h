
#pragma once

#include "CoreMinimal.h"
#include "Stats/Stats.h"
#include "Widgets/SWidget.h"
#include "EdGraph/EdGraphPin.h"
#include "BlueprintEditor.h"
#include "Animation/UnLive2DAnimNodeBase.h"

class UUnLive2DRendererComponent;

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
};

class FUnLive2DAnimationBlueprintEditor : public FBlueprintEditor
{
protected:

	typedef FBlueprintEditor Super;

public:

	void InitUnLive2DAnimationBlueprintEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, class UUnLive2DAnimBlueprint* InAnimBlueprint);

public:
	FUnLive2DAnimationBlueprintEditor();

	virtual ~FUnLive2DAnimationBlueprintEditor();

public:
	/** 更新显示当前选择信息的检查器*/
	void SetDetailObjects(const TArray<UObject*>& InObjects);
	void SetDetailObject(UObject* Obj);

	/** 处理常规对象选择 */
	void HandleObjectsSelected(const TArray<UObject*>& InObjects);
	void HandleObjectSelected(UObject* InObject);

protected:
	/** Undo Action**/
	void UndoAction();

	/** Redo Action **/
	void RedoAction();

protected:
	/** FBlueprintEdi1tor interface */
	virtual void CreateDefaultCommands() override;
	virtual void OnCreateGraphEditorCommands(TSharedPtr<FUICommandList> GraphEditorCommandsList);
	virtual bool CanSelectBone() const override { return true; }
	virtual void CreateDefaultTabContents(const TArray<UBlueprint*>& InBlueprints) override;
	virtual FGraphAppearanceInfo GetGraphAppearance(class UEdGraph* InGraph) const override;
	virtual bool IsEditable(UEdGraph* InGraph) const override;
	virtual FText GetGraphDecorationString(UEdGraph* InGraph) const override;

	virtual void OnActiveTabChanged(TSharedPtr<SDockTab> PreviouslyActive, TSharedPtr<SDockTab> NewlyActivated) override;
	virtual void OnSelectedNodesChangedImpl(const TSet<class UObject*>& NewSelection) override;
	virtual void HandleSetObjectBeingDebugged(UObject* InObject) override;

protected:
	// IToolkit Interface
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;

protected:
	//~ Begin FEditorUndoClient Interface
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;
	// End of FEditorUndoClient

	/** 调用post compile来复制节点数据*/
	void OnPostCompile();

	/** 用于使控件在预览和实例中保持同步的助手函数 */
	FUnLive2DAnimNode_Base* FindAnimNode(class UUnLive2DAnimGraphNode_Base* AnimGraphNode) const;

protected:

	//~ Begin FTickableEditorObject Interface
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	//~ End FTickableEditorObject Interface

	// IToolkit interface
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	// End of IToolkit interface

public:

	/** Returns a pointer to the Blueprint object we are currently editing, as long as we are editing exactly one */
	virtual UBlueprint* GetBlueprintObj() const override;

	//~ Begin FNotifyHook Interface
	virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged) override;
	//~ End FNotifyHook Interface

private:

	void BindCommands();

public:

	/** Handle the viewport being created */
	void HandleViewportCreated(const TSharedRef<class SCompoundWidget>& InPersonaViewport);

private:

	/** Extend menu */
	void ExtendMenu();

	/** Extend toolbar */
	void ExtendToolbar();

	/** Called immediately prior to a blueprint compilation */
	void OnBlueprintPreCompile(UBlueprint* BlueprintToCompile);

public:
	/** Delegate called after an undo operation for child widgets to refresh */
	FSimpleMulticastDelegate OnPostUndo;

private:

	TSharedPtr<class IUnLive2DToolkit> UnLive2DManagerToolkit;

	/** The extender to pass to the level editor to extend it's window menu */
	TSharedPtr<FExtender> MenuExtender;

	/** Toolbar extender */
	TSharedPtr<FExtender> ToolbarExtender;

	// 选择的动画蓝图
	TWeakObjectPtr<class UUnLive2DAnimGraphNode_Base> SelectedAnimGraphNode;

	/** The last pin type we added to a graph's inputs */
	FEdGraphPinType LastGraphPinType;

	UUnLive2DRendererComponent* DebuggedUnLive2DComponent;

	UUnLive2DAnimBlueprint* UnLive2DAnimBlueprintEdited;
};