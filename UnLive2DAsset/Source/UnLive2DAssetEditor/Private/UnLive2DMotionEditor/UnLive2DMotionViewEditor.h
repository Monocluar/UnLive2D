#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"
#include "EditorUndoClient.h"

class UUnLive2DAnimBase;

class FUnLive2DAnimBaseViewEditor : public FWorkflowCentricApplication, public FGCObject, public FEditorUndoClient
{
public:
	FUnLive2DAnimBaseViewEditor();

public:
	// 初始化编辑器
	void InitUnLive2DAnimViewEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, UUnLive2DAnimBase* InitUnLive2DAnimBase);

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

	UUnLive2DAnimBase* GetUnLive2DAnimBaseEdited() const {return UnLive2DAnimBeingEdited; }

	void SetUnLive2DAnimBeingEdited(UUnLive2DAnimBase* NewAnimBase);


	TWeakObjectPtr<UUnLive2DRendererComponent> GetUnLive2DRenderComponent() const;

private:
	// Live2D动作
	UUnLive2DAnimBase* UnLive2DAnimBeingEdited;

	/** 工具栏扩展器 */
	TSharedPtr<class FExtender> ToolbarExtender;

	/** UnLive2D资源管理器 */
	TSharedPtr<class IUnLive2DToolkit> UnLive2DAnimToolkit;

	// UnLive2D动作资源列表
	TSharedPtr<class SUnLive2DAnimBaseAssetBrowser> UnLive2DAnimAssetListPtr;


	TSharedPtr<class SUnLive2DAnimBaseEditorViewport> ViewportPtr;

public:
	/** 在全局撤消/重做时触发多播委托*/
	FSimpleMulticastDelegate OnPostUndo;

protected:
	// 绑定按键命令
	void BindCommands();
	void ExtendToolbar();

	void UpDataAnimBase();

	TSharedRef<SDockTab> SpawnTab_Viewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_AssetBrowser(const FSpawnTabArgs& Args);

	TSharedPtr<SDockTab> OpenNewAnimBaseDocumentTab(UUnLive2DAnimBase* InAnimBase);
};