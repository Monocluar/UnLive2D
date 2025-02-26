
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UnLive2DAnimBlueprintGraphNode_Base.h"
#include "UnLive2DAnimBlueprintGraphNode_Root.generated.h"


UCLASS(MinimalAPI)
class UUnLive2DAnimBlueprintGraphNode_Root : public UUnLive2DAnimBlueprintGraphNode_Base
{
	GENERATED_UCLASS_BODY()

protected:
	// UEdGraphNode interface
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual bool CanUserDeleteNode() const override { return false; }
	virtual bool CanDuplicateNode() const override { return false; }
	virtual void GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;
	// End of UEdGraphNode interface

	// UUnLive2DAnimBlueprintGraphNode_Base interface
	virtual void CreateInputPins() override;
	virtual bool IsRootNode() const override { return true; }
	// End of UUnLive2DAnimBlueprintGraphNode_Base interface
};