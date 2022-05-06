#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "EdGraph/EdGraphNode.h"
#include "UnLive2DAnimBlueprintGraphNode_Base.generated.h"

class UEdGraphPin;

UCLASS(MinimalAPI)
class UUnLive2DAnimBlueprintGraphNode_Base : public UEdGraphNode
{
	GENERATED_UCLASS_BODY()

public:
	// 创建所需的所有输入引脚
	virtual void  CreateInputPins() {};
	/** 是否是根节点 */
	virtual bool IsRootNode() const { return false; }

public:
	// 获取输出引脚
	UNLIVE2DASSETEDITOR_API UEdGraphPin* GetOutputPin();
	// 获取所有输入引脚
	UNLIVE2DASSETEDITOR_API void GetInputPins(TArray<class UEdGraphPin*>& OutInputPins);
	// 通过索引获取单个输入引脚
	UNLIVE2DASSETEDITOR_API UEdGraphPin* GetGetInputPin(int32 InputIndex);
	/** 获取输入引脚数 */
	UNLIVE2DASSETEDITOR_API int32 GetInputCount() const;

	
	/**
	 * 处理在FromPin和FromPin最初连接的对象之间插入节点
	 *
	 * @param FromPin			从中生成此节点的pin
	 * @param NewLinkPin		FromPin将连接到的新插脚
	 * @param OutNodeList		任何被修改的节点都将被添加到此列表中以进行通知
	 */
	void InsertNewNode(UEdGraphPin* FromPin, UEdGraphPin* NewLinkPin, TSet<UEdGraphNode*>& OutNodeList);

protected:
	// UEdGraphNode interface.
	virtual void AllocateDefaultPins() override;
	virtual void ReconstructNode() override;
	virtual void AutowireNewNode(UEdGraphPin* FromPin) override;
	virtual bool CanCreateUnderSpecifiedSchema(const UEdGraphSchema* Schema) const override;
	virtual FString GetDocumentationLink() const override;
	// End of UEdGraphNode interface.
};