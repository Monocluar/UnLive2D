#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "ITransportControl.h"
#include "EditorUndoClient.h"
#include "Framework/Commands/UICommandList.h"
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"

class SDockTab;
class UUnLive2D;

class FUnLive2DViewEditor : public FWorkflowCentricApplication, public FGCObject, public FEditorUndoClient
{
public:

	FUnLive2DViewEditor();



public:
	// 初始化编辑器
	void InitUnLive2DViewEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, UUnLive2D* InitUnLive2D);

public:
	// IToolkit interface
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	// End of IToolkit interface

	/** FEditorUndoClient interface */
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;

	
	//~ Begin FAssetEditorToolkit Interface.
	virtual bool OnRequestClose() override;
	//~ End FAssetEditorToolkit Interface.

public:
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
	virtual FString GetReferencerName() const override;
	// End of FSerializableObject interface

public:

	UUnLive2D* GetUnLive2DBeingEdited() const { return UnLive2DBeingEdited; }

protected:

	TSharedRef<SDockTab> SpawnTab_Viewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);

protected:
	// 打开的资源指针
	UUnLive2D* UnLive2DBeingEdited;

	/** 工具栏扩展器 */
	TSharedPtr<FExtender> ToolbarExtender;

	TSharedPtr<class SUnLive2DEditorViewport> ViewportPtr;

	/** UnLive2D资源管理器 */
	TSharedPtr<class IUnLive2DToolkit> UnLive2DToolkit;

	// Range of times currently being viewed
	mutable float ViewInputMin;
	mutable float ViewInputMax;
	mutable float LastObservedSequenceLength;

	
public:
	/** 在全局撤消/重做时触发多播委托*/
	FSimpleMulticastDelegate OnPostUndo;

protected:
	// 序列总长度
	float GetTotalSequenceLength() const;

protected:
	// 绑定按键命令
	void BindCommands();
	void ExtendToolbar();

};