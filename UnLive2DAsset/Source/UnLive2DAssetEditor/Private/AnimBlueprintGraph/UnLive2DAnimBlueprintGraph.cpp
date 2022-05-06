#include "AnimBlueprintGraph/UnLive2DAnimBlueprintGraph.h"
#include "Animation/UnLive2DAnimBlueprint.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintGraphSchema.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintGraphNode.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintNode_Base.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintGraphNode_Root.h"

class FUnLive2DAnimBlueprintAnimEditor : public IUnLive2DAnimBlueprintAnimEditor
{
public:
	FUnLive2DAnimBlueprintAnimEditor() {}

	virtual ~FUnLive2DAnimBlueprintAnimEditor() {}



protected:

	virtual UEdGraph* CreateUnLive2DAnimBlueprintGraph(UUnLive2DAnimBlueprint* InAnimBlueprint) override
	{
		UUnLive2DAnimBlueprintGraph* AnimBlueprintGraph = CastChecked<UUnLive2DAnimBlueprintGraph>(FBlueprintEditorUtils::CreateNewGraph(InAnimBlueprint, NAME_None, UUnLive2DAnimBlueprintGraph::StaticClass(), UUnLive2DAnimBlueprintGraphSchema::StaticClass()));

		return AnimBlueprintGraph;
	}

	virtual void RemoveNullNodes(UUnLive2DAnimBlueprint* InAnimBlueprint) override
	{
		for (int32 idx = InAnimBlueprint->GetGraph()->Nodes.Num() - 1; idx >= 0; --idx)
		{
			UUnLive2DAnimBlueprintGraphNode* Node = Cast<UUnLive2DAnimBlueprintGraphNode>(InAnimBlueprint->GetGraph()->Nodes[idx]);

			if (Node && Node->AnimBlueprintNode == NULL)
			{
				FBlueprintEditorUtils::RemoveNode(NULL, Node, true);
			}
		}
	}


	virtual void CreateInputPin(UEdGraphNode* AnimBlueprintNode) override
	{
		CastChecked<UUnLive2DAnimBlueprintGraphNode>(AnimBlueprintNode)->CreateInputPin();
	}


	virtual void SetupAnimBlueprintNode(UEdGraph* AnimBlueprintGraph, UUnLive2DAnimBlueprintNode_Base* InAnimBlueprintNode, bool bSelectNewNode) override
	{
		FGraphNodeCreator<UUnLive2DAnimBlueprintGraphNode> NodeCreator(*AnimBlueprintGraph);
		UUnLive2DAnimBlueprintGraphNode* GraphNode = NodeCreator.CreateNode(bSelectNewNode);
		GraphNode->SetAnimBlueprintNode(InAnimBlueprintNode);
	}


	virtual void LinkGraphNodesFromAnimBlueprintNodes(UUnLive2DAnimBlueprint* AnimBlueprint) override
	{
		if (AnimBlueprint->FirstNode != nullptr)
		{
			// Find the root node
			TArray<UUnLive2DAnimBlueprintGraphNode_Root*> RootNodeList;
			AnimBlueprint->GetGraph()->GetNodesOfClass<UUnLive2DAnimBlueprintGraphNode_Root>(/*out*/ RootNodeList);
			check(RootNodeList.Num() == 1);

			RootNodeList[0]->Pins[0]->BreakAllPinLinks();
			RootNodeList[0]->Pins[0]->MakeLinkTo(CastChecked<UUnLive2DAnimBlueprintGraphNode>(AnimBlueprint->FirstNode->GetGraphNode())->GetOutputPin());
		}

		for (TArray<UUnLive2DAnimBlueprintNode_Base*>::TConstIterator It(AnimBlueprint->GetGraphAllNodes()); It; ++It)
		{
			UUnLive2DAnimBlueprintNode_Base* AnimBlueprintNode = *It;
			if (AnimBlueprintNode == nullptr) continue;

			TArray<UEdGraphPin*> InputPins;
			CastChecked<UUnLive2DAnimBlueprintGraphNode>(AnimBlueprintNode->GetGraphNode())->GetInputPins(/*out*/ InputPins);
			check(InputPins.Num() == AnimBlueprintNode->ChildNodes.Num());
			for (int32 ChildIndex = 0; ChildIndex < AnimBlueprintNode->ChildNodes.Num(); ChildIndex++)
			{
				UUnLive2DAnimBlueprintNode_Base* ChildNode = AnimBlueprintNode->ChildNodes[ChildIndex];
				if (ChildNode == nullptr) continue;

				InputPins[ChildIndex]->BreakAllPinLinks();
				InputPins[ChildIndex]->MakeLinkTo(CastChecked<UUnLive2DAnimBlueprintGraphNode>(ChildNode->GetGraphNode())->GetOutputPin());
			}
		}
	}


	virtual void CompileAnimBlueprintNodesFromGraphNodes(UUnLive2DAnimBlueprint* AnimBlueprint) override
	{
		// Use GraphNodes to make UUnLive2DAnimBlueprintNode_Base Connections
		TArray<UUnLive2DAnimBlueprintNode_Base*> ChildNodes;
		TArray<UEdGraphPin*> InputPins;

		for (int32 NodeIndex = 0; NodeIndex < AnimBlueprint->GetGraph()->Nodes.Num(); ++NodeIndex)
		{
			UUnLive2DAnimBlueprintGraphNode* GraphNode = Cast<UUnLive2DAnimBlueprintGraphNode>(AnimBlueprint->GetGraph()->Nodes[NodeIndex]);
			if (GraphNode && GraphNode->AnimBlueprintNode)
			{
				// Set ChildNodes of each UUnLive2DAnimBlueprintNode_Base

				GraphNode->GetInputPins(InputPins);
				ChildNodes.Empty();
				for (int32 PinIndex = 0; PinIndex < InputPins.Num(); ++PinIndex)
				{
					UEdGraphPin* ChildPin = InputPins[PinIndex];

					if (ChildPin->LinkedTo.Num() > 0)
					{
						UUnLive2DAnimBlueprintGraphNode* GraphChildNode = CastChecked<UUnLive2DAnimBlueprintGraphNode>(ChildPin->LinkedTo[0]->GetOwningNode());
						ChildNodes.Add(GraphChildNode->AnimBlueprintNode);
					}
					else
					{
						ChildNodes.AddZeroed();
					}
				}

				GraphNode->AnimBlueprintNode->SetFlags(RF_Transactional);
				GraphNode->AnimBlueprintNode->Modify();
				GraphNode->AnimBlueprintNode->SetChildNodes(ChildNodes);
				GraphNode->AnimBlueprintNode->PostEditChange();
			}
			else
			{
				// Set FirstNode based on RootNode connection
				UUnLive2DAnimBlueprintGraphNode_Root* RootNode = Cast<UUnLive2DAnimBlueprintGraphNode_Root>(AnimBlueprint->GetGraph()->Nodes[NodeIndex]);
				if (RootNode)
				{
					AnimBlueprint->Modify();
					if (RootNode->Pins[0]->LinkedTo.Num() > 0)
					{
						AnimBlueprint->FirstNode = CastChecked<UUnLive2DAnimBlueprintGraphNode>(RootNode->Pins[0]->LinkedTo[0]->GetOwningNode())->AnimBlueprintNode;
					}
					else
					{
						AnimBlueprint->FirstNode = nullptr;
					}
					AnimBlueprint->PostEditChange();
				}
			}
		}
	}


	virtual void RenameNodePins(UUnLive2DAnimBlueprintNode_Base* AnimBlueprintNode) override
	{
		TArray<class UEdGraphPin*> InputPins;

#if WITH_EDITOR
		CastChecked<UUnLive2DAnimBlueprintGraphNode>(AnimBlueprintNode->GetGraphNode())->GetInputPins(InputPins);
#endif

		for (int32 i = 0; i < InputPins.Num(); i++)
		{
			if (InputPins[i])
			{
				InputPins[i]->PinName = *AnimBlueprintNode->GetInputPinName(i).ToString();
			}
		}
	}
};

UUnLive2DAnimBlueprintGraph::UUnLive2DAnimBlueprintGraph(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (!UUnLive2DAnimBlueprint::GetUnLive2DAnimBlueprintAnimEditor().IsValid())
	{
		UUnLive2DAnimBlueprint::SeUnLive2DAnimBlueprintAnimEditor(TSharedPtr<IUnLive2DAnimBlueprintAnimEditor>(new FUnLive2DAnimBlueprintAnimEditor()));
	}
}

UUnLive2DAnimBlueprint* UUnLive2DAnimBlueprintGraph::GetUnLive2DAnimBlueprint() const
{
	return CastChecked<UUnLive2DAnimBlueprint>(GetOuter());
}
