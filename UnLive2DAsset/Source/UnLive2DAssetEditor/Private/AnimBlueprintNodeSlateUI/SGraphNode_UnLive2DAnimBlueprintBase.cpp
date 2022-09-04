#include "SGraphNode_UnLive2DAnimBlueprintBase.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintGraphNode.h"
#include "GraphEditorSettings.h"

#define LOCTEXT_NAMESPACE "UnLive2D"

void SGraphNode_UnLive2DAnimBlueprintBase::Construct(const FArguments& InArgs, class UUnLive2DAnimBlueprintGraphNode* InNode)
{
	GraphNode = InNode;
	UnLive2DAnimBlueprintGraphNode = InNode;

	SetCursor(EMouseCursor::CardinalCross);

	UpdateGraphNode();
}

void SGraphNode_UnLive2DAnimBlueprintBase::CreateOutputSideAddButton(TSharedPtr<SVerticalBox> OutputBox)
{
	TSharedRef<SWidget> AddPinButton = AddPinButtonContent(
		LOCTEXT("UnLive2DAnimBlueprintNodeAddPinButton", "Add input"),
		LOCTEXT("UnLive2DAnimBlueprintNodeAddPinButton_Tooltip", "Adds an input to the UnLive2D AnimBlueprint node")
	);

	FMargin AddPinPadding = Settings->GetOutputPinPadding();
	AddPinPadding.Top += 6.0f;

	OutputBox->AddSlot()
		.AutoHeight()
		.VAlign(VAlign_Center)
		.Padding(AddPinPadding)
		[
			AddPinButton
		];
}

EVisibility SGraphNode_UnLive2DAnimBlueprintBase::IsAddPinButtonVisible() const
{
	EVisibility ButtonVisibility = SGraphNode::IsAddPinButtonVisible();
	if (ButtonVisibility == EVisibility::Visible)
	{
		if (!UnLive2DAnimBlueprintGraphNode->CanAddInputPin())
		{
			ButtonVisibility = EVisibility::Collapsed;
		}
	}
	return ButtonVisibility;
}

FReply SGraphNode_UnLive2DAnimBlueprintBase::OnAddPin()
{
	UnLive2DAnimBlueprintGraphNode->AddInputPin();

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE