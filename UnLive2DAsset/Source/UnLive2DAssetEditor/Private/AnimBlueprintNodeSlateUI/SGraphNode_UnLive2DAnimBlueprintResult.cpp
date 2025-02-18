#include "SGraphNode_UnLive2DAnimBlueprintResult.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintGraphNode_Root.h"
#include "EditorStyleSet.h"
#include "UnLive2DCubismCore.h"

void SGraphNode_UnLive2DAnimBlueprintResult::Construct(const FArguments& InArgs, class UUnLive2DAnimBlueprintGraphNode_Root* InNode)
{
	GraphNode = InNode;

	SetCursor(EMouseCursor::CardinalCross);

	UpdateGraphNode();
}

TSharedRef<SWidget> SGraphNode_UnLive2DAnimBlueprintResult::CreateNodeContentArea()
{
	return SNew(SBorder)
		.BorderImage(FUnLive2DStyle::GetBrush("NoBorder"))
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.Padding(FMargin(0, 3))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.FillWidth(1.0f)
			[
				// LEFT
				SAssignNew(LeftNodeBox, SVerticalBox)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.Image(FUnLive2DStyle::GetBrush("Graph.AnimationResultNode.Body"))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			[
				// RIGHT
				SAssignNew(RightNodeBox, SVerticalBox)
			]
		];
}

