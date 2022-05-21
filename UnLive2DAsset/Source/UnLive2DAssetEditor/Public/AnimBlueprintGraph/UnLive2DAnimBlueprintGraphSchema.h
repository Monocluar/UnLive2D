
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
	// 获取当前选定的节点组
	static FGraphPanelSelectionSet GetSelectedNodes(const UEdGraph* Graph);

};


UCLASS(MinimalAPI)
class UUnLive2DAnimBlueprintGraphSchema : public UEdGraphSchema
{
	GENERATED_UCLASS_BODY()

public:
	// 尝试将多个节点的输出连接到单个节点的输入
	void TryConnectNodes(const TArray<UUnLive2DAnimBlueprintNode_Base*>& OutputNodes, UUnLive2DAnimBlueprintNode_Base* InputNode) const;

public:

	//~ Begin EdGraphSchema Interface
	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;
	virtual bool TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const override;
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