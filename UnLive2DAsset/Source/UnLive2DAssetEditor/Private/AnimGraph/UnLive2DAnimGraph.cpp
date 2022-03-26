#include "AnimGraph/UnLive2DAnimGraph.h"
#include "RigVMModel/RigVMGraph.h"
#if WITH_EDITOR
#include "Animation/UnLive2DAnimDefines.h"
#endif
#include "RigVMModel/RigVMController.h"

UUnLive2DAnimGraph::UUnLive2DAnimGraph()
{
	bSuspendModelNotifications = false;
	bIsSelecting = false;
}

UEdGraphNode* UUnLive2DAnimGraph::FindNodeForModelNodeName(const FName& InModelNodeName)
{
	for (UEdGraphNode* EdNode : Nodes)
	{
		if (UUnLive2DAnimGraphNode_Base* AnimNode = Cast<UUnLive2DAnimGraphNode_Base>(EdNode))
		{
			if (AnimNode->ModelNodePath == InModelNodeName.ToString())
			{
				return EdNode;
			}
		}
		else
		{
			if (EdNode->GetFName() == InModelNodeName)
			{
				return EdNode;
			}
		}
	}

	return nullptr;
}

void UUnLive2DAnimGraph::HandleModifiedEvent(ERigVMGraphNotifType InNotifType, URigVMGraph* InGraph, UObject* InSubject)
{
	if (bSuspendModelNotifications)
	{
		return;
	}

	switch (InNotifType)
	{
	case ERigVMGraphNotifType::GraphChanged:
		{
			for (URigVMNode* Node : InGraph->GetNodes())
			{
				UEdGraphNode* EdNode = FindNodeForModelNodeName(Node->GetFName());
				if (EdNode != nullptr)
				{
					RemoveNode(EdNode);
				}
			}
		}
		break;

	case ERigVMGraphNotifType::NodeSelectionChanged:
		{
			if (bIsSelecting)
			{
				return;
			}
			TGuardValue<bool> SelectionGuard(bIsSelecting, true);
			TSet<const UEdGraphNode*> NodeSelection;
			for (FName NodeName : InGraph->GetSelectNodes())
			{
				if (UEdGraphNode* EdNode = FindNodeForModelNodeName(NodeName))
				{
					NodeSelection.Add(EdNode);
				}
			}
			SelectNodeSet(NodeSelection);
		}
		break;
	case ERigVMGraphNotifType::NodeAdded:
		{
			if (URigVMNode* ModelNode = Cast<URigVMNode>(InSubject))
			{

			}
		}
		break;
	}
}

#if WITH_EDITOR

URigVMController* UUnLive2DAnimGraph::GetTemplateController()
{
	if (TemplateModel == nullptr)
	{
		TemplateModel = NewObject<URigVMGraph>(this, TEXT("TemplateModel"));
	}
	if (TemplateController == nullptr)
	{
		TemplateController = NewObject<URigVMController>(this, TEXT("TemplateController"));
		TemplateController->SetExecuteContextStruct(FUnLive2DAnimeExecuteContext::StaticStruct());
		TemplateController->SetGraph(TemplateModel);
		TemplateController->EnableReporting(false);
		TemplateController->OnModified().AddUObject(this, &UUnLive2DAnimGraph::HandleModifiedEvent);

		TemplateController->SetFlags(RF_Transient);
		TemplateModel->SetFlags(RF_Transient);
	}
	return TemplateController;
}
#endif


