
#pragma once

#include "CoreMinimal.h"
#include "EdGraphUtilities.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintGraphNode_Base.h"
#include "AnimBlueprintNodeSlateUI/SGraphNode_UnLive2DAnimBlueprintResult.h"
#include "AnimBlueprintNodeSlateUI/SGraphNode_UnLive2DAnimBlueprintBase.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintGraphNode_Root.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintGraphNode.h"


class FUnLive2DAnimBlueprintGraphNodeFactory : public FGraphPanelNodeFactory
{
	virtual TSharedPtr<class SGraphNode> CreateNode(class UEdGraphNode* InNode) const override
	{
		if (UUnLive2DAnimBlueprintGraphNode_Base* BaseUnLive2DAnimBlueprintNode = Cast<UUnLive2DAnimBlueprintGraphNode_Base>(InNode))
		{
			if (UUnLive2DAnimBlueprintGraphNode_Root* RootUnLive2DAnimBlueprintNode = Cast<UUnLive2DAnimBlueprintGraphNode_Root>(InNode))
			{
				return SNew(SGraphNode_UnLive2DAnimBlueprintResult, RootUnLive2DAnimBlueprintNode);
			}
			else if (UUnLive2DAnimBlueprintGraphNode* UnLive2DAnimBlueprintNode = Cast<UUnLive2DAnimBlueprintGraphNode>(InNode))
			{
				return SNew(SGraphNode_UnLive2DAnimBlueprintBase, UnLive2DAnimBlueprintNode);
			}
		}
		return nullptr;
	}
};