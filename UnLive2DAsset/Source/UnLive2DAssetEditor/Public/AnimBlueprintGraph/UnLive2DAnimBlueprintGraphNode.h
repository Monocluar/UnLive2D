
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UnLive2DAnimBlueprintGraphNode_Base.h"
#include "UnLive2DAnimBlueprintGraphNode.generated.h"


class UUnLive2DAnimBlueprintNode_Base;

UCLASS(MinimalAPI)
class UUnLive2DAnimBlueprintGraphNode : public UUnLive2DAnimBlueprintGraphNode_Base
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(VisibleAnywhere, Instanced, Category=UnLive2DAnimBlueprint)
		UUnLive2DAnimBlueprintNode_Base* AnimBlueprintNode;

public:
	// 设置它代表的AnimBlueprintNode（在编辑器中将其指定给AnimBlueprintNode）
	UNLIVE2DASSETEDITOR_API void SetAnimBlueprintNode(UUnLive2DAnimBlueprintNode_Base* InAnimBlueprintNode);
	// 复制后修复节点的所有者
	UNLIVE2DASSETEDITOR_API void PostCopyNode();
	// 为此节点创建新的Pin
	UNLIVE2DASSETEDITOR_API void CreateInputPin();
	// 根据标题的长度估计此节点的宽度
	UNLIVE2DASSETEDITOR_API int32 EstimateNodeWidth() const;
	// 将输入引脚添加到此节点并重新编译
	UNLIVE2DASSETEDITOR_API void AddInputPin();
	// 从此节点中删除特定的输入引脚并重新编译
	UNLIVE2DASSETEDITOR_API void RemoveInputPin(UEdGraphPin* InGraphPin);
	// 是否可以添加节点
	UNLIVE2DASSETEDITOR_API bool CanAddInputPin() const;

protected:
	// UUnLive2DAnimBlueprintGraphNode_Base interface
	virtual void CreateInputPins() override;
	// End of UUnLive2DAnimBlueprintGraphNode_Base interface

	// UEdGraphNode interface
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual void PrepareForCopying() override;
	virtual void GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;
	virtual FText GetTooltipText() const override;
	virtual FString GetDocumentationExcerptName() const override;
	// End of UEdGraphNode interface

	friend class FUnLive2DAnimationBlueprintEditor;

private:
	// 返回节点的所有权
	void ResetUnLive2DAnimBlueprintNodeOwner();
};