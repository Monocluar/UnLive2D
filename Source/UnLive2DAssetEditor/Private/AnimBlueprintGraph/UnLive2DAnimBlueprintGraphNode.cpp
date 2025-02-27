#include "AnimBlueprintGraph/UnLive2DAnimBlueprintGraphNode.h"
#include "ToolMenuDelegates.h"
#include "ToolMenuSection.h"
#include "Framework/Commands/GenericCommands.h"
#include "ToolMenu.h"
#include "GraphEditorActions.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintNode_Base.h"
#include "Engine/Font.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintGraph.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintEditorCommands.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintNode_MotionPlayer.h"

#define LOCTEXT_NAMESPACE "UnLive2DAnimBlueprinGraphNode"

UUnLive2DAnimBlueprintGraphNode::UUnLive2DAnimBlueprintGraphNode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UUnLive2DAnimBlueprintGraphNode::SetAnimBlueprintNode(UUnLive2DAnimBlueprintNode_Base* InAnimBlueprintNode)
{
	AnimBlueprintNode = InAnimBlueprintNode;
	AnimBlueprintNode->GraphNode = this;
}

void UUnLive2DAnimBlueprintGraphNode::PostCopyNode()
{
	ResetUnLive2DAnimBlueprintNodeOwner();
}

void UUnLive2DAnimBlueprintGraphNode::CreateInputPin()
{
	UEdGraphPin* NewPin = CreatePin(EGPD_Input, TEXT("UnLive2DAnimBlueprintNode"), *AnimBlueprintNode->GetInputPinName(GetInputCount()).ToString());
	if (NewPin->PinName.IsNone())
	{
		// 确保pin有一个用于查找的名称，但用户永远不会看到它
		NewPin->PinName = CreateUniquePinName(TEXT("Input"));
		NewPin->PinFriendlyName = FText::FromString(TEXT(" "));
	}
}

int32 UUnLive2DAnimBlueprintGraphNode::EstimateNodeWidth() const
{
	const int32 EstimatedCharWidth = 6;
	FString NodeTitle = GetNodeTitle(ENodeTitleType::FullTitle).ToString();
	UFont* Font = GetDefault<UEditorEngine>()->EditorFont;
	int32 Result = NodeTitle.Len() * EstimatedCharWidth;

	if (Font)
	{
		Result = Font->GetStringSize(*NodeTitle);
	}

	return Result;
}

void UUnLive2DAnimBlueprintGraphNode::AddInputPin()
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "SoundCueEditorAddInput", "Add Sound Cue Input"));

	Modify();
	CreateInputPin();

	UUnLive2DAnimBlueprint* UnLive2DAnimBlueprint = CastChecked<UUnLive2DAnimBlueprintGraph>(GetGraph())->GetUnLive2DAnimBlueprint();
	UnLive2DAnimBlueprint->CompileUnLive2DAnimNodesFromGraphNodes();
	UnLive2DAnimBlueprint->MarkPackageDirty();

	// 刷新当前图形，以便可以更新管脚
	GetGraph()->NotifyGraphChanged();
}

void UUnLive2DAnimBlueprintGraphNode::RemoveInputPin(UEdGraphPin* InGraphPin)
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "UnLive2DAnimBlueprintEditorDeleteInput", "Delete UnLive2D AnimBlueprint Input"));
	Modify();

	TArray<class UEdGraphPin*> InputPins;
	GetInputPins(InputPins);

	for (int32 InputIndex = 0; InputIndex < InputPins.Num(); InputIndex++)
	{
		if (InGraphPin == InputPins[InputIndex])
		{
#if ENGINE_MAJOR_VERSION < 5
			InGraphPin->MarkPendingKill();
#else
			InGraphPin->MarkAsGarbage();
#endif
			Pins.Remove(InGraphPin);
			AnimBlueprintNode->Modify();
			AnimBlueprintNode->RemoveChildNode(InputIndex);
			break;
		}
	}

	UUnLive2DAnimBlueprint* UnLive2DAnimBlueprint = CastChecked<UUnLive2DAnimBlueprintGraph>(GetGraph())->GetUnLive2DAnimBlueprint();
	UnLive2DAnimBlueprint->CompileUnLive2DAnimNodesFromGraphNodes();
	UnLive2DAnimBlueprint->MarkPackageDirty();

	// Refresh the current graph, so the pins can be updated
	GetGraph()->NotifyGraphChanged();
}

bool UUnLive2DAnimBlueprintGraphNode::CanAddInputPin() const
{
	if (AnimBlueprintNode)
	{
		// 子节点数小于最大子节点数则可以添加节点
		return AnimBlueprintNode->ChildNodes.Num() < AnimBlueprintNode->GetMaxChildNodes();
	}

	return false;
}

void UUnLive2DAnimBlueprintGraphNode::CreateInputPins()
{
	if (AnimBlueprintNode)
	{
		for (int32 ChildIndex = 0; ChildIndex < AnimBlueprintNode->ChildNodes.Num(); ++ChildIndex)
		{
			CreateInputPin();
		}
	}
}

FText UUnLive2DAnimBlueprintGraphNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (AnimBlueprintNode)
	{
		return AnimBlueprintNode->GetTitle();
	}
	else
	{
		return Super::GetNodeTitle(TitleType);
	}
}

void UUnLive2DAnimBlueprintGraphNode::PrepareForCopying()
{
	if (AnimBlueprintNode)
	{
		// 暂时拥有AnimBlueprintNode的所有权，以便在剪切时不会删除它
		AnimBlueprintNode->Rename(NULL, this, REN_DontCreateRedirectors);
	}
}

void UUnLive2DAnimBlueprintGraphNode::GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const
{
	if (Context->Pin)
	{
		// 如果输入的内容可以删除，请显示选项
		if (Context->Pin->Direction == EGPD_Input && AnimBlueprintNode->ChildNodes.Num() > AnimBlueprintNode->GetMinChildNodes())
		{
			FToolMenuSection& Section = Menu->AddSection("UnLive2DAnimBlueprintGraphDeleteInput");
			Section.AddMenuEntry(FUnLive2DAnimBlueprintEditorCommands::Get().DeleteInput);
		}
	}
	else if (Context->Node)
	{
		{
			FToolMenuSection& Section = Menu->AddSection("UnLive2DAnimBlueprintGraphNodeAlignment");
			Section.AddSubMenu("Alignment", LOCTEXT("AlignmentHeader", "Alignment"), FText(), FNewToolMenuDelegate::CreateLambda([](UToolMenu* SubMenu)
			{
				{
					FToolMenuSection& SubMenuSection = SubMenu->AddSection("EdGraphSchemaAlignment", LOCTEXT("AlignHeader", "Align"));
					SubMenuSection.AddMenuEntry(FGraphEditorCommands::Get().AlignNodesTop);
					SubMenuSection.AddMenuEntry(FGraphEditorCommands::Get().AlignNodesMiddle);
					SubMenuSection.AddMenuEntry(FGraphEditorCommands::Get().AlignNodesBottom);
					SubMenuSection.AddMenuEntry(FGraphEditorCommands::Get().AlignNodesLeft);
					SubMenuSection.AddMenuEntry(FGraphEditorCommands::Get().AlignNodesCenter);
					SubMenuSection.AddMenuEntry(FGraphEditorCommands::Get().AlignNodesRight);
					SubMenuSection.AddMenuEntry(FGraphEditorCommands::Get().StraightenConnections);
				}

				{
					FToolMenuSection& SubMenuSection = SubMenu->AddSection("EdGraphSchemaDistribution", LOCTEXT("DistributionHeader", "Distribution"));
					SubMenuSection.AddMenuEntry(FGraphEditorCommands::Get().DistributeNodesHorizontally);
					SubMenuSection.AddMenuEntry(FGraphEditorCommands::Get().DistributeNodesVertically);
				}
			}));
		}

		{
			FToolMenuSection& Section = Menu->AddSection("UnLive2DAnimBlueprintGraphNodeEdit");
			Section.AddMenuEntry(FGenericCommands::Get().Delete);
			Section.AddMenuEntry(FGenericCommands::Get().Cut);
			Section.AddMenuEntry(FGenericCommands::Get().Copy);
			Section.AddMenuEntry(FGenericCommands::Get().Duplicate);
		}

		{
			FToolMenuSection& Section = Menu->AddSection("SUnLive2DAnimBlueprintGraphNodeAddPlaySync");
			if (CanAddInputPin())
			{
				Section.AddMenuEntry(FUnLive2DAnimBlueprintEditorCommands::Get().AddInput);
			}
			Section.AddMenuEntry(FUnLive2DAnimBlueprintEditorCommands::Get().PlayNode);

			if (Cast<UUnLive2DAnimBlueprintNode_MotionPlayer>(AnimBlueprintNode))
			{
				Section.AddMenuEntry(FUnLive2DAnimBlueprintEditorCommands::Get().BrowserSync);
			}
			/*else if (Cast<UUnLive2DAnimBlueprintNode_ExpressionPlayer>(AnimBlueprintNode))
			{
				Section.AddMenuEntry(FUnLive2DAnimBlueprintEditorCommands::Get().BrowserSync);
			}*/
		}
	}
}

FText UUnLive2DAnimBlueprintGraphNode::GetTooltipText() const
{
	FText Tooltip;
	if (AnimBlueprintNode)
	{
		Tooltip = AnimBlueprintNode->GetClass()->GetToolTipText();
	}
	if (Tooltip.IsEmpty())
	{
		Tooltip = GetNodeTitle(ENodeTitleType::ListView);
	}
	return Tooltip;
}

FString UUnLive2DAnimBlueprintGraphNode::GetDocumentationExcerptName() const
{
	UClass* MyClass = (AnimBlueprintNode != NULL) ? AnimBlueprintNode->GetClass() : this->GetClass();
	return FString::Printf(TEXT("%s%s"), MyClass->GetPrefixCPP(), *MyClass->GetName());
}

void UUnLive2DAnimBlueprintGraphNode::PostLoad()
{
	Super::PostLoad();

	if (AnimBlueprintNode)
	{
		AnimBlueprintNode->GraphNode = this;
	}

	for (int32 i = 0; i < Pins.Num(); i++)
	{
		UEdGraphPin* Pin = Pins[i];
		if (!Pin->PinName.IsNone()) continue;

		// Makes sure pin has a name for lookup purposes but user will never see it
		if (Pin->Direction == EGPD_Input)
		{
			Pin->PinName = CreateUniquePinName(TEXT("Input"));
		}
		else
		{
			Pin->PinName = CreateUniquePinName(TEXT("Output"));
		}
		Pin->PinFriendlyName = FText::FromString(TEXT(" "));
	}
}

void UUnLive2DAnimBlueprintGraphNode::PostEditImport()
{
	ResetUnLive2DAnimBlueprintNodeOwner();
}

void UUnLive2DAnimBlueprintGraphNode::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);

	if (!bDuplicateForPIE)
	{
		CreateNewGuid();
	}
}

void UUnLive2DAnimBlueprintGraphNode::ResetUnLive2DAnimBlueprintNodeOwner()
{
	if (AnimBlueprintNode == nullptr) return;

	UUnLive2DAnimBlueprint* UnLive2DAnimBlueprint = CastChecked<UUnLive2DAnimBlueprintGraph>(GetGraph())->GetUnLive2DAnimBlueprint();

	if (AnimBlueprintNode->GetOuter() != UnLive2DAnimBlueprint)
	{
		AnimBlueprintNode->Rename(NULL, UnLive2DAnimBlueprint, REN_DontCreateRedirectors);
	}

	AnimBlueprintNode->GraphNode = this;
}

#undef LOCTEXT_NAMESPACE