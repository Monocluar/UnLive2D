#pragma once

#include "CoreMinimal.h"
#include "IUnLive2DParameterEditorAsset.h"

class UUnLive2DAnimBase;

class FUnLive2DAnimBaseViewEditor : public IUnLive2DParameterEditorAsset
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

	virtual UUnLive2D* GetUnLive2DBeingEdited() const override;
	virtual TWeakObjectPtr<UUnLive2DRendererComponent> GetUnLive2DRenderComponent() const override;

	virtual EUnLive2DParameterAssetType::Type GetUnLive2DParameterAssetType() const override;

	virtual bool GetUnLive2DParameterHasSaveData() const  override;

	virtual bool GetUnLive2DParameterAddParameterData() const override;

private:
	// Live2D动作
	UUnLive2DAnimBase* UnLive2DAnimBeingEdited;

	/** 工具栏扩展器 */
	TSharedPtr<class FExtender> ToolbarExtender;

	/** UnLive2D资源管理器 */
	TSharedPtr<class IUnLive2DToolkit> UnLive2DAnimToolkit;

	// UnLive2D动作资源列表
	TSharedPtr<class SUnLive2DAnimBaseAssetBrowser> UnLive2DAnimAssetListPtr;

	// 视口
	TSharedPtr<class SUnLive2DAnimBaseEditorViewport> ViewportPtr;

	// UnLive2D动作参数组
	TSharedPtr<class SUnLive2DParameterGroup> UnLive2DParameterGroupPtr;

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
	TSharedRef<SDockTab> SpawnTab_ParameterGroup(const FSpawnTabArgs& Args);

	TSharedPtr<SDockTab> OpenNewAnimBaseDocumentTab(UUnLive2DAnimBase* InAnimBase);
};