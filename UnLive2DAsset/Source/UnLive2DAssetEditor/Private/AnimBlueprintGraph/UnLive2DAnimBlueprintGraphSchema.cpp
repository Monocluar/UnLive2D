#include "AnimBlueprintGraph/UnLive2DAnimBlueprintGraphSchema.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintNode_Base.h"
#include "Editor.h"
#include "UnLive2DMotion.h"
#include "EdGraphNode_Comment.h"
#include "EdGraph/EdGraph.h"
#include "Editor.h"
#include "Engine/Selection.h"
#include "UObject/Class.h"
#include "UnLive2DAnimBlueprint/UnLive2DAnimationBlueprintEditor.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintGraph.h"
#include "Toolkits/ToolkitManager.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintGraphNode.h"

#define LOCTEXT_NAMESPACE "UnLive2D"

TArray<UClass*> UUnLive2DAnimBlueprintGraphSchema::UnLive2DAnimNodeClasses;
bool UUnLive2DAnimBlueprintGraphSchema::bUnLive2DAnimNodeClassesInitialized = false;

TSharedPtr<FUnLive2DAnimationBlueprintEditor> GetUnLie2DAnimBlueprintEditorForObject(const UObject* ObjectToFocusOn)
{
	check(ObjectToFocusOn);

	UUnLive2DAnimBlueprint* UnLive2DAnimBlueprint = Cast<const UUnLive2DAnimBlueprintGraph>(ObjectToFocusOn)->GetUnLive2DAnimBlueprint();

	TSharedPtr<FUnLive2DAnimationBlueprintEditor> UnLive2DAnimationBlueprintEditor;
	if (UnLive2DAnimBlueprint)
	{
		TSharedPtr< IToolkit > FoundAssetEditor = FToolkitManager::Get().FindEditorForAsset(UnLive2DAnimBlueprint);
		if (FoundAssetEditor.IsValid())
		{
			UnLive2DAnimationBlueprintEditor = StaticCastSharedPtr<FUnLive2DAnimationBlueprintEditor>(FoundAssetEditor);
		}
	}

	return UnLive2DAnimationBlueprintEditor;
}

UUnLive2DAnimBlueprintGraphSchema::UUnLive2DAnimBlueprintGraphSchema(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UUnLive2DAnimBlueprintGraphSchema::TryConnectNodes(const TArray<UUnLive2DAnimBlueprintNode_Base*>& OutputNodes, UUnLive2DAnimBlueprintNode_Base* InputNode) const
{
	for (int32 Index = 0; Index < OutputNodes.Num(); Index++)
	{
		if (Index < InputNode->GetMaxChildNodes())
		{
			UUnLive2DAnimBlueprintGraphNode* GraphNode = CastChecked<UUnLive2DAnimBlueprintGraphNode>(InputNode->GetGraphNode());
			if (Index >= GraphNode->GetInputCount())
			{
				GraphNode->CreateInputPin();
			}
			TryCreateConnection(GraphNode->GetInputPin(Index), CastChecked<UUnLive2DAnimBlueprintGraphNode>(OutputNodes[Index]->GetGraphNode())->GetOutputPin());
		}
	}
}

void UUnLive2DAnimBlueprintGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
}

bool UUnLive2DAnimBlueprintGraphSchema::TryCreateConnection(UEdGraphPin* PinA, UEdGraphPin* PinB) const
{
	bool bModified = UEdGraphSchema::TryCreateConnection(PinA, PinB);

	if (bModified)
	{
		CastChecked<UUnLive2DAnimBlueprintGraph>(PinA->GetOwningNode()->GetGraph())->GetUnLive2DAnimBlueprint()->CompileUnLive2DAnimNodesFromGraphNodes();
	}

	return bModified;
}

void UUnLive2DAnimBlueprintGraphSchema::GetAllUnLive2DAnimNodeActions(FGraphActionMenuBuilder& ActionMenuBuilder, bool bShowSelectedActions) const
{
	InitUnLive2DAnimNodeClasses();

	FText SelectedItemText;
	bool IsMotionSelected = false;

	if (bShowSelectedActions)
	{
		FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();

		// 获取可能选择的任何项目的显示文本
		if (ActionMenuBuilder.FromPin == NULL)
		{
			TArray<UUnLive2DMotion*> SelectedMotion;
			GEditor->GetSelectedObjects()->GetSelectedObjects<UUnLive2DMotion>(SelectedMotion);
			int32 TotalAnim = SelectedMotion.Num();

			if (TotalAnim > 1)
			{
				SelectedItemText = LOCTEXT("MultipleAnimsSelected", "Multiple Anims");
			}
			else if (SelectedMotion.Num() == 1)
			{
				SelectedItemText = FText::FromString(SelectedMotion[0]->GetName());
				IsMotionSelected = true;
			}
		}
		else
		{
			UUnLive2DMotion* SelectedMotion =  GEditor->GetSelectedObjects()->GetTop<UUnLive2DMotion>();
			if (SelectedMotion && ActionMenuBuilder.FromPin->Direction == EGPD_Input)
			{
				SelectedItemText = FText::FromString(SelectedMotion->GetName());
				IsMotionSelected = true;
			}
		}

		bShowSelectedActions = !SelectedItemText.IsEmpty();
	}

	for (UClass* UnLive2DAnimNodeClass : UnLive2DAnimNodeClasses)
	{
		UUnLive2DAnimBlueprintNode_Base* AnimBlueprintNode = UnLive2DAnimNodeClass->GetDefaultObject<UUnLive2DAnimBlueprintNode_Base>();

		// 从输出引脚拖动时，可以创建节点任何内容
		if (!ActionMenuBuilder.FromPin || ActionMenuBuilder.FromPin->Direction == EGPD_Input || AnimBlueprintNode->GetMaxChildNodes() > 0)
		{
			const FText Name = FText::FromString(UnLive2DAnimNodeClass->GetDescription());

			{
				FFormatNamedArguments Arguments;
				Arguments.Add(TEXT("Name"), Name);
				const FText AddToolTip = FText::Format(LOCTEXT("NewUnLive2DAnimNodeTooltip", "Adds {Name} node here"), Arguments);
				TSharedPtr<FUnLive2DAnimBlueprintGraphSchemaAction_NewNode> NewNodeAction(new FUnLive2DAnimBlueprintGraphSchemaAction_NewNode(LOCTEXT("UnLive2DAnimBlueprintNodeAction", "UnLive2D AnimBlueprint Node"), Name, AddToolTip, 0));
			}
		}
	}

}

void UUnLive2DAnimBlueprintGraphSchema::GetCommentAction(FGraphActionMenuBuilder& ActionMenuBuilder, const UEdGraph* CurrentGraph /*= NULL*/) const
{

}

void UUnLive2DAnimBlueprintGraphSchema::InitUnLive2DAnimNodeClasses()
{
	if (bUnLive2DAnimNodeClassesInitialized) return;

	// 构造非抽象UnLive2DAnimNode类的列表
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->IsChildOf(UUnLive2DAnimBlueprintNode_Base::StaticClass()) && !It->HasAnyClassFlags(CLASS_Abstract))
		{
			UnLive2DAnimNodeClasses.Add(*It);
		}
	}
	UnLive2DAnimNodeClasses.Sort();

	bUnLive2DAnimNodeClassesInitialized = true;
}

UEdGraphNode* FUnLive2DAnimBlueprintGraphSchemaAction_NewNode::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode /*= true*/)
{
	check(UnLive2DAnimBlueprintNodeClass);

	UUnLive2DAnimBlueprint* AnimBlueprint = CastChecked<UUnLive2DAnimBlueprintGraph>(ParentGraph)->GetUnLive2DAnimBlueprint();
	const FScopedTransaction Transaction(LOCTEXT("UnLive2DAnimBlueprintEditorNewAnimNode", "UnLive2D AnimBlueprint Editor: New Anim Node"));
	ParentGraph->Modify();
	AnimBlueprint->Modify();

	UUnLive2DAnimBlueprintNode_Base* NewNode = AnimBlueprint->ConstructAnimNode<UUnLive2DAnimBlueprintNode_Base>(UnLive2DAnimBlueprintNodeClass, bSelectNewNode);

	// If this node allows >0 children but by default has zero - create a connector for starters
	if (NewNode->GetMaxChildNodes() > 0 && NewNode->ChildNodes.Num() == 0)
	{
		NewNode->CreateStartingConnectors();
	}

	// Attempt to connect inputs to selected nodes, unless we're already dragging from a single output
	if (FromPin == NULL || FromPin->Direction == EGPD_Input)
	{
		ConnectToSelectedNodes(NewNode, ParentGraph);
	}

	NewNode->GraphNode->NodePosX = Location.X;
	NewNode->GraphNode->NodePosY = Location.Y;

	NewNode->GraphNode->AutowireNewNode(FromPin);

	AnimBlueprint->PostEditChange();
	AnimBlueprint->MarkPackageDirty();

	return NewNode->GraphNode;
}

void FUnLive2DAnimBlueprintGraphSchemaAction_NewNode::ConnectToSelectedNodes(UUnLive2DAnimBlueprintNode_Base* NewNodeClass, UEdGraph* ParentGraph) const
{
	// 仅当节点可以有多个子节点时才连接
	if (NewNodeClass->GetMaxChildNodes() > 1)
	{
		const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes(ParentGraph);

		TArray<UUnLive2DAnimBlueprintNode_Base*> SortedNodes;
		for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
		{
			UUnLive2DAnimBlueprintGraphNode* SelectedNode = Cast<UUnLive2DAnimBlueprintGraphNode>(*NodeIt);

			if (SelectedNode == nullptr) continue;

			bool bInserted = false;
			// 按y轴位置对节点排序
			for (int32 Index = 0; Index < SortedNodes.Num(); ++Index)
			{
				if (SortedNodes[Index]->GraphNode->NodePosY > SelectedNode->NodePosY)
				{
					SortedNodes.Insert(SelectedNode->AnimBlueprintNode, Index);
					bInserted = true;
					break;
				}
			}
			if (!bInserted)
			{
				SortedNodes.Add(SelectedNode->AnimBlueprintNode);
			}
		}
		if (SortedNodes.Num() > 1)
		{
			CastChecked<UUnLive2DAnimBlueprintGraphSchema>(NewNodeClass->GetGraphNode()->GetSchema())->TryConnectNodes(SortedNodes, NewNodeClass);
		}
	}
}

FGraphPanelSelectionSet FUnLive2DAnimBlueprintGraphSchemaAction_NewNode::GetSelectedNodes(const UEdGraph* Graph)
{
	TSharedPtr<FUnLive2DAnimationBlueprintEditor> AnimBlueprintEditor = GetUnLie2DAnimBlueprintEditorForObject(Graph);
	if (AnimBlueprintEditor.IsValid())
	{
		return AnimBlueprintEditor->GetSelectedNodes();
	}
	return TSet<UObject*>();
}

#undef LOCTEXT_NAMESPACE