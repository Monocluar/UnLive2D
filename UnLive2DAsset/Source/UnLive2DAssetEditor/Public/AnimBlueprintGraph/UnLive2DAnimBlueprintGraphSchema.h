
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "AssetData.h"
#include "EdGraph/EdGraphSchema.h"
#include "UnLive2DAnimBlueprintGraphSchema.generated.h"

class UUnLive2DAnimBlueprintNode_Base;

USTRUCT()
struct UNLIVE2DASSETEDITOR_API FUnLive2DAnimBlueprintGraphSchemaAction_NewNode : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY()

public:
	/** 要创建的节点的类 */
	UPROPERTY()
		class UClass* UnLive2DAnimBlueprintNodeClass;

public:
	FUnLive2DAnimBlueprintGraphSchemaAction_NewNode()
		: FEdGraphSchemaAction()
		, UnLive2DAnimBlueprintNodeClass(nullptr)
	{}

	FUnLive2DAnimBlueprintGraphSchemaAction_NewNode(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping)
		, UnLive2DAnimBlueprintNodeClass(nullptr)
	{}

protected:

	//~ Begin FEdGraphSchemaAction Interface
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	//~ End FEdGraphSchemaAction Interface
private:
	/** Connects new node to output of selected nodes */
	void ConnectToSelectedNodes(UUnLive2DAnimBlueprintNode_Base* NewNodeClass, UEdGraph* ParentGraph) const;

public:
	// 是否可以粘贴内容到该图标中?
	static bool CanPasteNodes(const class UEdGraph* Graph);

	// 在位置对图表执行粘贴
	static void  PasteNodesHere(class UEdGraph* Graph, const FVector2D& Location);

	// 基于选定对象添加UnLive2DAnimBlueprintNode_MotionPlayer节点
	static void CreateMotionPlayerContainers(TArray<class UUnLive2DMotion*>& SelectedMotionPlayer, class UUnLive2DAnimBlueprint* AnimBlueprint, TArray<class UUnLive2DAnimBlueprintNode_Base*>& OutPlayers, FVector2D Location);

	// 获取当前选中的节点组的最大边界
	static bool GetBoundsForSelectedNodes(const UEdGraph* Graph, class FSlateRect& Rect, float Padding = 0.0f);

	// 获取当前选定的节点数
	static int32 GetNumberOfSelectedNodes(const UEdGraph* Graph);

	// 获取当前选定的节点组
	static FGraphPanelSelectionSet GetSelectedNodes(const UEdGraph* Graph);

};

// 粘贴图表数据
USTRUCT()
struct UNLIVE2DASSETEDITOR_API FUnLive2DAnimBlueprintGraphSchemaAction_Paste : public FUnLive2DAnimBlueprintGraphSchemaAction_NewNode
{
	GENERATED_USTRUCT_BODY();

public:
	FUnLive2DAnimBlueprintGraphSchemaAction_Paste()
		: FUnLive2DAnimBlueprintGraphSchemaAction_NewNode()
	{}

	FUnLive2DAnimBlueprintGraphSchemaAction_Paste(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping)
		: FUnLive2DAnimBlueprintGraphSchemaAction_NewNode(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping)
	{}

protected:
	//~ Begin FEdGraphSchemaAction Interface
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	//~ End FEdGraphSchemaAction Interface
};

// 基于选定对象将节点添加到图形的操作
USTRUCT()
struct UNLIVE2DASSETEDITOR_API FUnLive2DAnimBlueprintGraphSchemaAction_NewFromSelected : public FUnLive2DAnimBlueprintGraphSchemaAction_NewNode
{
	GENERATED_USTRUCT_BODY();

public:
	FUnLive2DAnimBlueprintGraphSchemaAction_NewFromSelected()
		: FUnLive2DAnimBlueprintGraphSchemaAction_NewNode()
	{}

	FUnLive2DAnimBlueprintGraphSchemaAction_NewFromSelected(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping)
		: FUnLive2DAnimBlueprintGraphSchemaAction_NewNode(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping)
	{}

protected:
	//~ Begin FEdGraphSchemaAction Interface
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	//~ End FEdGraphSchemaAction Interface
};

// 创建新注释的操作
USTRUCT()
struct UNLIVE2DASSETEDITOR_API FUnLive2DAnimBlueprintGraphSchemaAction_NewComment : public FUnLive2DAnimBlueprintGraphSchemaAction_NewNode
{
	GENERATED_USTRUCT_BODY();

public:
	FUnLive2DAnimBlueprintGraphSchemaAction_NewComment()
		: FUnLive2DAnimBlueprintGraphSchemaAction_NewNode()
	{}

	FUnLive2DAnimBlueprintGraphSchemaAction_NewComment(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping)
		: FUnLive2DAnimBlueprintGraphSchemaAction_NewNode(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping)
	{}

protected:
	//~ Begin FEdGraphSchemaAction Interface
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	//~ End FEdGraphSchemaAction Interface
};


UCLASS(MinimalAPI)
class UUnLive2DAnimBlueprintGraphSchema : public UEdGraphSchema
{
	GENERATED_UCLASS_BODY()

public:
	// 检查连接这些针脚是否会导致循环
	bool ConnectionCausesLoop(const UEdGraphPin* InputPin, const UEdGraphPin* OutputPin) const;

	// 尝试将多个节点的输出连接到单个节点的输入
	void TryConnectNodes(const TArray<UUnLive2DAnimBlueprintNode_Base*>& OutputNodes, UUnLive2DAnimBlueprintNode_Base* InputNode) const;

public:

	//~ Begin EdGraphSchema Interface
	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;
	virtual void GetContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;
	virtual void CreateDefaultNodesForGraph(UEdGraph& Graph) const override;
	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;
	virtual bool TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const override;
	virtual bool ShouldHidePinDefaultValue(UEdGraphPin* Pin) const override { return true; };
	virtual FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override { return  FLinearColor::White; };
	virtual void BreakNodeLinks(UEdGraphNode& TargetNode) const override;
	virtual void BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const override;
	virtual void GetAssetsGraphHoverMessage(const TArray<FAssetData>& Assets, const UEdGraph* HoverGraph, FString& OutTooltipText, bool& OutOkIcon) const override;
	virtual void DroppedAssetsOnGraph(const TArray<struct FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraph* Graph) const override;
	virtual void DroppedAssetsOnNode(const TArray<FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraphNode* Node) const override;
	virtual int32 GetNodeSelectionCount(const UEdGraph* Graph) const override;
	virtual TSharedPtr<FEdGraphSchemaAction> GetCreateCommentAction() const override;
	//~ End EdGraphSchema Interface

private:

	/** 添加用于创建每种类型的UnLive2DAnimNode的操作 */
	void GetAllUnLive2DAnimNodeActions(FGraphActionMenuBuilder& ActionMenuBuilder, bool bShowSelectedActions) const;
	/** 添加用于创建注释的操作 */
	void GetCommentAction(FGraphActionMenuBuilder& ActionMenuBuilder, const UEdGraph* CurrentGraph = NULL) const;

	// 生成所有可用UnLive2DAnimNode类的列表
	static void InitUnLive2DAnimNodeClasses();

private:
	// 所有可用的UnLive2DAnimNode类
	static TArray<UClass*> UnLive2DAnimNodeClasses;
	// 是否已填充UnLive2DAnimNode类列表
	static bool bUnLive2DAnimNodeClassesInitialized;
};