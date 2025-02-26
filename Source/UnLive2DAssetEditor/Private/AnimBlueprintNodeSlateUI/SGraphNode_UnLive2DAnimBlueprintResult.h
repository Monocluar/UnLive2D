#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"

class SGraphNode_UnLive2DAnimBlueprintResult : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SGraphNode_UnLive2DAnimBlueprintResult) {}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, class UUnLive2DAnimBlueprintGraphNode_Root* InNode);

protected:
	// SGraphNode interface
	virtual TSharedRef<SWidget> CreateNodeContentArea() override;
	// End of SGraphNode interface
};