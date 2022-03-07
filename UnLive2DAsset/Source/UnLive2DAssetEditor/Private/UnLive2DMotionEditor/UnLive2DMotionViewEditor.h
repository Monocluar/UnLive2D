#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"
#include "EditorUndoClient.h"

class UUnLive2DMotion;

class FUnLive2DMotionViewEditor : public FWorkflowCentricApplication, public FGCObject, public FEditorUndoClient
{
public:
	FUnLive2DMotionViewEditor();

public:
	// 初始化编辑器
	void InitUnLive2DMotionViewEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, UUnLive2DMotion* InitUnLive2DMotion);

protected:
	// IToolkit interface
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	// End of IToolkit interface

	/** FEditorUndoClient interface */
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;

	// FAssetEditorToolkit
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FString GetDocumentationLink() const override;
	// End of FAssetEditorToolkit

public:
	// FSerializableObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	// End of FSerializableObject interface

public:

	UUnLive2DMotion* GetUnLive2DMotionEdited() const {return UnLive2DMotionBeingEdited; }

	void SetUnLive2DMotionBeingEdited(UUnLive2DMotion* NewMotion);

private:
	// Live2D动作
	UUnLive2DMotion* UnLive2DMotionBeingEdited;

	/** 工具栏扩展器 */
	TSharedPtr<class FExtender> ToolbarExtender;

	/** UnLive2D资源管理器 */
	TSharedPtr<class IUnLive2DToolkit> UnLive2DMotionToolkit;

	// UnLive2D动作资源列表
	TSharedPtr<class SUnLive2DMotionAssetBrowser> UnLive2DMotionAssetListPtr;


	TSharedPtr<class SUnLive2DMotionEditorViewport> ViewportPtr;

public:
	/** 在全局撤消/重做时触发多播委托*/
	FSimpleMulticastDelegate OnPostUndo;

protected:
	// 绑定按键命令
	void BindCommands();
	void ExtendToolbar();

	void UpDataMotion();

	TSharedRef<SDockTab> SpawnTab_Viewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_AssetBrowser(const FSpawnTabArgs& Args);

	TSharedPtr<SDockTab> OpenNewMotionDocumentTab(UUnLive2DMotion* InMotion);
};