#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Toolkits/AssetEditorManager.h"
#include "ITransportControl.h"
#include "EditorUndoClient.h"
#include "Framework/Commands/UICommandList.h"

class SDockTab;
class UUnLive2D;

class FUnLive2DViewEditor : public FAssetEditorToolkit, public FGCObject
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
	// End of FSerializableObject interface

public:

	UUnLive2D* GetUnLive2DBeingEdited() const { return UnLive2DBeingEdited; }

protected:

	TSharedRef<SDockTab> SpawnTab_Viewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);

protected:
	// 打开的资源指针
	UUnLive2D* UnLive2DBeingEdited;

	TSharedPtr<class SUnLive2DEditorViewport> ViewportPtr;

	// 选择集
	int32 CurrentSelectedKeyframe;

	// Range of times currently being viewed
	mutable float ViewInputMin;
	mutable float ViewInputMax;
	mutable float LastObservedSequenceLength;

protected:
	// 序列总长度
	float GetTotalSequenceLength() const;

protected:
	// 绑定按键命令
	void BindCommands();
	void ExtendToolBar();

	void CreateModeToolbarWidgets(FToolBarBuilder& ToolbarBuilder);

};