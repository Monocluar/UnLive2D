#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"

class SGraphNode_UnLive2DAnimBlueprintBase : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SGraphNode_UnLive2DAnimBlueprintBase) {}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, class UUnLive2DAnimBlueprintGraphNode* InNode);

protected:
	// SGraphNode Interface
	virtual void CreateOutputSideAddButton(TSharedPtr<SVerticalBox> OutputBox) override;
	virtual EVisibility IsAddPinButtonVisible() const override;
	virtual FReply OnAddPin() override;

private:
	UUnLive2DAnimBlueprintGraphNode* UnLive2DAnimBlueprintGraphNode;
};