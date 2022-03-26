#include "AnimGraph/UnLive2DAnimStateEntryNode.h"

UUnLive2DAnimStateEntryNode::UUnLive2DAnimStateEntryNode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UUnLive2DAnimStateEntryNode::AllocateDefaultPins()
{
	UEdGraphPin* Outputs = CreatePin(EGPD_Output, TEXT("exec"), TEXT("Entry"));
}

FText UUnLive2DAnimStateEntryNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	UEdGraph* Graph = GetGraph();
	return FText::FromString(Graph->GetName());
}

FText UUnLive2DAnimStateEntryNode::GetTooltipText() const
{
	return NSLOCTEXT("AnimStateEntryNode", "StateEntryNodeTooltip", "Entry point for state machine");
}

UEdGraphNode* UUnLive2DAnimStateEntryNode::GetOutputNode() const
{
	if (Pins.Num() > 0 && Pins[0] != NULL)
	{
		check(Pins[0]->LinkedTo.Num() <= 1);
		if (Pins[0]->LinkedTo.Num() > 0 && Pins[0]->LinkedTo[0]->GetOwningNode() != NULL)
		{
			return Pins[0]->LinkedTo[0]->GetOwningNode();
		}
	}
	return NULL;
}

