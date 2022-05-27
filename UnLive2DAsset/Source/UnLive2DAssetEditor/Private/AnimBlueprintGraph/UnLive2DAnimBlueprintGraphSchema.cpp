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
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintNode_MotionPlayer.h"
#include "EdGraph/EdGraphNode.h"
#include "ToolMenu.h"
#include "ToolMenuSection.h"
#include "GraphEditorActions.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintGraphNode_Root.h"

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

bool UUnLive2DAnimBlueprintGraphSchema::ConnectionCausesLoop(const UEdGraphPin* InputPin, const UEdGraphPin* OutputPin) const
{
	UUnLive2DAnimBlueprintGraphNode* InputNode = Cast<UUnLive2DAnimBlueprintGraphNode>(InputPin->GetOwningNode());

	if (InputNode)
	{
		// 只有表示AnimBlueprintNode的节点具有输出
		UUnLive2DAnimBlueprintGraphNode* OutputNode = CastChecked<UUnLive2DAnimBlueprintGraphNode>(OutputPin->GetOwningNode());

		if (OutputNode->AnimBlueprintNode)
		{
			// 获取所有子节点，因为循环可能在任何子节点中
			TArray<UUnLive2DAnimBlueprintNode_Base*> Nodes;
			OutputNode->AnimBlueprintNode->GetAllNodes(Nodes);

			// 如果循环节点在输入节点中，则是循环节点
			return Nodes.Contains(InputNode->AnimBlueprintNode);
		}
	}

	return false;
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
	GetAllUnLive2DAnimNodeActions(ContextMenuBuilder, true);

	GetCommentAction(ContextMenuBuilder, ContextMenuBuilder.CurrentGraph);

	if (!ContextMenuBuilder.FromPin && FUnLive2DAnimBlueprintGraphSchemaAction_NewNode::CanPasteNodes(ContextMenuBuilder.CurrentGraph))
	{
		TSharedPtr<FUnLive2DAnimBlueprintGraphSchemaAction_Paste> NewAction(new FUnLive2DAnimBlueprintGraphSchemaAction_Paste(FText::GetEmpty(), LOCTEXT("PasteHereAction", "Paste here"), FText::GetEmpty(), 0));
		ContextMenuBuilder.AddAction(NewAction);
	}
}

void UUnLive2DAnimBlueprintGraphSchema::GetContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const
{
	if (Context->Node)
	{
		const UUnLive2DAnimBlueprintGraphNode* AnimBlueprintGraphNode = Cast<UUnLive2DAnimBlueprintGraphNode>(Context->Node);

		FToolMenuSection& Section = Menu->AddSection("AnimBlueprintGraphSchemaNodeActions", LOCTEXT("NodeActionsMenuHeader", "Node Actions"));
		Section.AddMenuEntry(FGraphEditorCommands::Get().BreakNodeLinks);
	}

	Super::GetContextMenuActions(Menu, Context);
}

void UUnLive2DAnimBlueprintGraphSchema::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	const int32 RootNodeHeightOffset = -58;

	// Create the result node
	FGraphNodeCreator<UUnLive2DAnimBlueprintGraphNode_Root> NodeCreator(Graph);
	UUnLive2DAnimBlueprintGraphNode_Root* ResultRootNode = NodeCreator.CreateNode();
	ResultRootNode->NodePosY = RootNodeHeightOffset;
	NodeCreator.Finalize();
	SetNodeMetaData(ResultRootNode, FNodeMetadata::DefaultGraphNode);
}

const FPinConnectionResponse UUnLive2DAnimBlueprintGraphSchema::CanCreateConnection(const UEdGraphPin* PinA, const UEdGraphPin* PinB) const
{
	// 确保针脚不在同一节点上
	if (PinA->GetOwningNode() == PinB->GetOwningNode())
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("ConnectionSameNode", "Both are on the same node"));
	}

	// 比较方向
	const UEdGraphPin* InputPin = NULL;
	const UEdGraphPin* OutputPin = NULL;

	if (!CategorizePinsByDirection(PinA, PinB, /*out*/ InputPin, /*out*/ OutputPin))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("ConnectionIncompatible", "Directions are not compatible"));
	}

	if (ConnectionCausesLoop(InputPin, OutputPin))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("ConnectionLoop", "Connection would cause loop"));
	}

	// 仅断开输入端的现有连接-可接受多个输出连接
	if (InputPin->LinkedTo.Num() > 0)
	{
		ECanCreateConnectionResponse ReplyBreakOutputs;
		if (InputPin == PinA)
		{
			ReplyBreakOutputs = CONNECT_RESPONSE_BREAK_OTHERS_A;
		}
		else
		{
			ReplyBreakOutputs = CONNECT_RESPONSE_BREAK_OTHERS_B;
		}
		return FPinConnectionResponse(ReplyBreakOutputs, LOCTEXT("ConnectionReplace", "Replace existing connections"));
	}

	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
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

void UUnLive2DAnimBlueprintGraphSchema::BreakNodeLinks(UEdGraphNode& TargetNode) const
{
	Super::BreakNodeLinks(TargetNode);

	CastChecked<UUnLive2DAnimBlueprintGraph>(TargetNode.GetGraph())->GetUnLive2DAnimBlueprint()->CompileUnLive2DAnimNodesFromGraphNodes();
}

void UUnLive2DAnimBlueprintGraphSchema::BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_BreakPinLinks", "Break Pin Links"));
	Super::BreakPinLinks(TargetPin, bSendsNodeNotifcation);

	if (bSendsNodeNotifcation)
	{
		CastChecked<UUnLive2DAnimBlueprintGraph>(TargetPin.GetOwningNode()->GetGraph())->GetUnLive2DAnimBlueprint()->CompileUnLive2DAnimNodesFromGraphNodes();
	}
}

void UUnLive2DAnimBlueprintGraphSchema::GetAssetsGraphHoverMessage(const TArray<FAssetData>& Assets, const UEdGraph* HoverGraph, FString& OutTooltipText, bool& OutOkIcon) const
{
	OutOkIcon = false;

	for (int32 AssetIdx = 0; AssetIdx < Assets.Num(); ++AssetIdx)
	{
		UUnLive2DMotion* UnLive2DMotion =  Cast<UUnLive2DMotion>(Assets[AssetIdx].GetAsset());
		if (UnLive2DMotion)
		{
			OutOkIcon = true;
			return;
		}
	}
}

void UUnLive2DAnimBlueprintGraphSchema::DroppedAssetsOnGraph(const TArray<struct FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraph* Graph) const
{
	TArray<UUnLive2DMotion*> Motions;
	for (const FAssetData& Item : Assets)
	{
		if (UUnLive2DMotion* Motion = Cast<UUnLive2DMotion>(Item.GetAsset()))
		{
			Motions.Add(Motion);
		}
	}

	if (Motions.Num() > 0)
	{
		const FScopedTransaction Transaction(LOCTEXT("AnimBlueprintEditorDropMotion", "Anim Blueprint Editor: 拖动动画资源"));

		UUnLive2DAnimBlueprintGraph* UnLive2DAnimBlueprintGraph = CastChecked<UUnLive2DAnimBlueprintGraph>(Graph);
		UUnLive2DAnimBlueprint* UnLive2DAnimBlueprint = UnLive2DAnimBlueprintGraph->GetUnLive2DAnimBlueprint();

		UnLive2DAnimBlueprintGraph->Modify();

		TArray<UUnLive2DAnimBlueprintNode_Base*> CreatedPlayer;
		FUnLive2DAnimBlueprintGraphSchemaAction_NewNode::CreateMotionPlayerContainers(Motions, UnLive2DAnimBlueprint, CreatedPlayer, GraphPosition);
	}
}

void UUnLive2DAnimBlueprintGraphSchema::DroppedAssetsOnNode(const TArray<FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraphNode* Node) const
{
	// 目前，拖放仅支持拖放动画图节点
	if (!Node->IsA<UUnLive2DAnimBlueprintGraphNode>())
	{
		return;
	}

	UUnLive2DAnimBlueprintGraphNode* UnLive2DAnimBlueprintGraphNode = CastChecked<UUnLive2DAnimBlueprintGraphNode>(Node);
	UUnLive2DAnimBlueprintGraph* UnLive2DAnimBlueprintGraph = CastChecked<UUnLive2DAnimBlueprintGraph>(Node->GetGraph());
	UUnLive2DAnimBlueprint* UnLive2DAnimBlueprint = UnLive2DAnimBlueprintGraph->GetUnLive2DAnimBlueprint();

	TArray<UUnLive2DMotion*> Motions;
	for (const FAssetData& Item : Assets)
	{
		if (UUnLive2DMotion* Motion = Cast<UUnLive2DMotion>(Item.GetAsset()))
		{
			Motions.Add(Motion);
		}
	}

	UUnLive2DAnimBlueprintNode_MotionPlayer* MotionPlayer = Cast<UUnLive2DAnimBlueprintNode_MotionPlayer>(UnLive2DAnimBlueprintGraphNode->AnimBlueprintNode);
	if (MotionPlayer)
	{
		if (Motions.Num() > 0)
		{
			if (Motions.Num() >= 1)
			{
				UnLive2DAnimBlueprintGraph->Modify();
				MotionPlayer->SetUnLive2DMotion(Motions[0]);
			}


			TArray<UUnLive2DAnimBlueprintNode_Base*> CreatedPlayers;
			FUnLive2DAnimBlueprintGraphSchemaAction_NewNode::CreateMotionPlayerContainers(Motions, UnLive2DAnimBlueprint, CreatedPlayers, GraphPosition);
		}
	}

	UnLive2DAnimBlueprintGraph->NotifyGraphChanged();
}

int32 UUnLive2DAnimBlueprintGraphSchema::GetNodeSelectionCount(const UEdGraph* Graph) const
{
	return FUnLive2DAnimBlueprintGraphSchemaAction_NewNode::GetNumberOfSelectedNodes(Graph);
}

TSharedPtr<FEdGraphSchemaAction> UUnLive2DAnimBlueprintGraphSchema::GetCreateCommentAction() const
{
	return TSharedPtr<FEdGraphSchemaAction>(static_cast<FEdGraphSchemaAction*>(new FUnLive2DAnimBlueprintGraphSchemaAction_NewComment));
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
				ActionMenuBuilder.AddAction(NewNodeAction);
				NewNodeAction->UnLive2DAnimBlueprintNodeClass = UnLive2DAnimNodeClass;
			}

			if (bShowSelectedActions && 
				(AnimBlueprintNode->GetMaxChildNodes() == UUnLive2DAnimBlueprintNode_Base::MAX_ALLOWED_CHILD_NODES ||
					UnLive2DAnimNodeClass == UUnLive2DAnimBlueprintNode_MotionPlayer::StaticClass()))
			{
				FFormatNamedArguments Arguments;
				Arguments.Add(TEXT("Name"), Name);
				Arguments.Add(TEXT("SelectedItems"), SelectedItemText);
				const FText MenuDesc = FText::Format(LOCTEXT("NewUnLive2DAnimNodeRandom", "{Name}: {SelectedItems}"), Arguments);
				const FText ToolTip = FText::Format(LOCTEXT("NewUnLive2DAnimNodeTooltip", "Adds a {Name} node for {SelectedItems} here"), Arguments);
				TSharedPtr<FUnLive2DAnimBlueprintGraphSchemaAction_NewFromSelected> NewNodeAction(new FUnLive2DAnimBlueprintGraphSchemaAction_NewFromSelected(LOCTEXT("FromSelected", "From Selected"),
					MenuDesc,
					ToolTip, 0));
				ActionMenuBuilder.AddAction(NewNodeAction);
				NewNodeAction->UnLive2DAnimBlueprintNodeClass = (UnLive2DAnimNodeClass == UUnLive2DAnimBlueprintNode_MotionPlayer::StaticClass() ? NULL : UnLive2DAnimNodeClass);
			}
		}
	}

}

void UUnLive2DAnimBlueprintGraphSchema::GetCommentAction(FGraphActionMenuBuilder& ActionMenuBuilder, const UEdGraph* CurrentGraph /*= NULL*/) const
{
	if (ActionMenuBuilder.FromPin) return;

	const bool bIsManyNodesSelected = CurrentGraph ? (FUnLive2DAnimBlueprintGraphSchemaAction_NewNode::GetNumberOfSelectedNodes(CurrentGraph) > 0) : false;
	const FText MenuDescription = bIsManyNodesSelected ? LOCTEXT("CreateCommentAction", "Create Comment from Selection") : LOCTEXT("AddCommentAction", "Add Comment...");
	const FText ToolTip = LOCTEXT("CreateCommentToolTip", "Creates a comment.");

	TSharedPtr<FUnLive2DAnimBlueprintGraphSchemaAction_NewComment> NewAction(new FUnLive2DAnimBlueprintGraphSchemaAction_NewComment(FText::GetEmpty(), MenuDescription, ToolTip, 0));
	ActionMenuBuilder.AddAction(NewAction);

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

UEdGraphNode* FUnLive2DAnimBlueprintGraphSchemaAction_Paste::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode /*= true*/)
{
	PasteNodesHere(ParentGraph, Location);
	return nullptr;
}

UEdGraphNode* FUnLive2DAnimBlueprintGraphSchemaAction_NewFromSelected::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode /*= true*/)
{
	check(UnLive2DAnimBlueprintNodeClass);

	UUnLive2DAnimBlueprint* AnimBlueprint = CastChecked<UUnLive2DAnimBlueprintGraph>(ParentGraph)->GetUnLive2DAnimBlueprint();
	const FScopedTransaction Transaction(LOCTEXT("UnLive2DAnimBlueprintEditorNewAnimNode", "UnLive2D AnimBlueprint Editor: New Anim Node"));
	ParentGraph->Modify();
	AnimBlueprint->Modify();


	UEdGraphNode* CreatedNode = NULL;

	FVector2D AnimNodeStartLocation = Location;

	if (UnLive2DAnimBlueprintNodeClass)
	{
		// 如果要创建另一个节点，请将动画节点移开。
		AnimNodeStartLocation.X -= 200;
	}


	TArray<UUnLive2DMotion*> SelectedAnim;
	TArray<UUnLive2DAnimBlueprintNode_Base*> CreatedPlayers;

	GEditor->GetSelectedObjects()->GetSelectedObjects<UUnLive2DMotion>(SelectedAnim);

	CreateMotionPlayerContainers(SelectedAnim, AnimBlueprint, CreatedPlayers, AnimNodeStartLocation);

	if (UnLive2DAnimBlueprintNodeClass)
	{
		UUnLive2DAnimBlueprintNode_Base* NewNode = AnimBlueprint->ConstructAnimNode<UUnLive2DAnimBlueprintNode_Base>();
		UEdGraphNode* NewGraphNode = NewNode->GraphNode;
		const UUnLive2DAnimBlueprintGraphSchema* NewSchema = CastChecked<UUnLive2DAnimBlueprintGraphSchema>(NewGraphNode->GetSchema());

		// 如果此节点允许>0个子节点，但默认情况下子节点为零-为初学者创建一个连接器
		if (NewNode->GetMaxChildNodes() > 0 && NewNode->ChildNodes.Num() == 0)
		{
			NewNode->CreateStartingConnectors();
		}

		NewSchema->TryConnectNodes(CreatedPlayers, NewNode);

		NewGraphNode->NodePosX = Location.X;
		NewGraphNode->NodePosY = Location.Y;

		CreatedNode = NewNode->GraphNode;
	}
	else
	{
		if (CreatedPlayers.Num() > 0)
		{
			CreatedNode = CreatedPlayers[0]->GraphNode;
		}
	}

	if (CreatedNode)
	{
		CreatedNode->AutowireNewNode(FromPin);
	}

	AnimBlueprint->PostEditChange();
	AnimBlueprint->MarkPackageDirty();

	return CreatedNode;
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

bool FUnLive2DAnimBlueprintGraphSchemaAction_NewNode::CanPasteNodes(const class UEdGraph* Graph)
{
	bool bCanPaste = false;
	TSharedPtr<FUnLive2DAnimationBlueprintEditor> AnimBlueprintEditor = GetUnLie2DAnimBlueprintEditorForObject(Graph);
	if (AnimBlueprintEditor.IsValid())
	{
		bCanPaste = AnimBlueprintEditor->CanPasteNodes();
	}
	return bCanPaste;
}

void FUnLive2DAnimBlueprintGraphSchemaAction_NewNode::PasteNodesHere(class UEdGraph* Graph, const FVector2D& Location)
{
	TSharedPtr<FUnLive2DAnimationBlueprintEditor> AnimBlueprintEditor = GetUnLie2DAnimBlueprintEditorForObject(Graph);
	if (AnimBlueprintEditor.IsValid())
	{
		AnimBlueprintEditor->PasteNodesHere(Location);
	}

}

void FUnLive2DAnimBlueprintGraphSchemaAction_NewNode::CreateMotionPlayerContainers(TArray<UUnLive2DMotion*>& SelectedMotionPlayer, UUnLive2DAnimBlueprint* AnimBlueprint, TArray<UUnLive2DAnimBlueprintNode_Base*>& OutPlayers, FVector2D Location)
{
	const int32 NodeSpacing = 70; // 节点间距

	Location.Y -= static_cast<float>((SelectedMotionPlayer.Num() - 1) * NodeSpacing) / 2.f;

	for (int32 MotionIndex = 0; MotionIndex < SelectedMotionPlayer.Num(); MotionIndex++)
	{
		UUnLive2DMotion* NewMotion = SelectedMotionPlayer[MotionIndex];
		if (NewMotion == nullptr) continue;
		UUnLive2DAnimBlueprintNode_MotionPlayer* MotionPlayer = AnimBlueprint->ConstructAnimNode<UUnLive2DAnimBlueprintNode_MotionPlayer>();

		MotionPlayer->SetUnLive2DMotion(NewMotion);

		MotionPlayer->GraphNode->NodePosX = Location.X - CastChecked<UUnLive2DAnimBlueprintGraphNode>(MotionPlayer->GetGraphNode())->EstimateNodeWidth();
		MotionPlayer->GraphNode->NodePosY = Location.Y + (NodeSpacing * MotionIndex);

		OutPlayers.Add(MotionPlayer);
	}
}

bool FUnLive2DAnimBlueprintGraphSchemaAction_NewNode::GetBoundsForSelectedNodes(const UEdGraph* Graph, class FSlateRect& Rect, float Padding /*= 0.0f*/)
{
	TSharedPtr<FUnLive2DAnimationBlueprintEditor> AnimBlueprintEditor = GetUnLie2DAnimBlueprintEditorForObject(Graph);
	if (AnimBlueprintEditor.IsValid())
	{
		return AnimBlueprintEditor->GetBoundsForSelectedNodes(Rect, Padding);
	}
	return false;
}

int32 FUnLive2DAnimBlueprintGraphSchemaAction_NewNode::GetNumberOfSelectedNodes(const UEdGraph* Graph)
{
	TSharedPtr<FUnLive2DAnimationBlueprintEditor> AnimBlueprintEditor = GetUnLie2DAnimBlueprintEditorForObject(Graph);
	if (AnimBlueprintEditor.IsValid())
	{
		return AnimBlueprintEditor->GetNumberOfSelectedNodes();
	}
	return 0;
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

UEdGraphNode* FUnLive2DAnimBlueprintGraphSchemaAction_NewComment::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode /*= true*/)
{
	// 添加用于创建注释框的菜单项
	UEdGraphNode_Comment* CommentTemplate = NewObject<UEdGraphNode_Comment>();

	FVector2D SpawnLocation = Location;

	FSlateRect Bounds;
	if (GetBoundsForSelectedNodes(ParentGraph, Bounds, 50.f))
	{
		CommentTemplate->SetBounds(Bounds);
		SpawnLocation.X = CommentTemplate->NodePosX;
		SpawnLocation.Y = CommentTemplate->NodePosY;
	}

	FEdGraphSchemaAction_NewNode Action;
	Action.NodeTemplate = CommentTemplate;

	return Cast<UEdGraphNode_Comment>(Action.PerformAction(ParentGraph, nullptr, Location, bSelectNewNode));
}

#undef LOCTEXT_NAMESPACE
