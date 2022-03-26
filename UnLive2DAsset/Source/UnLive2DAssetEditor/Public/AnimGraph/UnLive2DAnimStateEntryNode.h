
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "EdGraph/EdGraphNode.h"
#include "UnLive2DAnimStateEntryNode.generated.h"

UCLASS(MinimalAPI)
class UUnLive2DAnimStateEntryNode : public UEdGraphNode
{
	GENERATED_UCLASS_BODY()

protected:
	//~ Begin UEdGraphNode Interface
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	//~ End UEdGraphNode Interface

public:
	UNLIVE2DASSETEDITOR_API UEdGraphNode* GetOutputNode() const;
};