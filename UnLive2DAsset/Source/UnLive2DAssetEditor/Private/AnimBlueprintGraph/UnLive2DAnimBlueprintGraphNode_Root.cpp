#include "AnimBlueprintGraph/UnLive2DAnimBlueprintGraphNode_Root.h"
#include "GraphEditorSettings.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintEditorCommands.h"
#include "ToolMenu.h"

#define LOCTEXT_NAMESPACE "UnLive2DAnimBlueprintGraphNode_Root"

UUnLive2DAnimBlueprintGraphNode_Root::UUnLive2DAnimBlueprintGraphNode_Root(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FLinearColor UUnLive2DAnimBlueprintGraphNode_Root::GetNodeTitleColor() const
{
	return GetDefault<UGraphEditorSettings>()->ResultNodeTitleColor;
}

FText UUnLive2DAnimBlueprintGraphNode_Root::GetTooltipText() const
{
	return LOCTEXT("RootToolTip", "将最终的动画蓝图节点连接到此节点");
}

FText UUnLive2DAnimBlueprintGraphNode_Root::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("RootTitle", "Output");
}

void UUnLive2DAnimBlueprintGraphNode_Root::GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const
{
	if (Context->Pin)
	{
	}
	else if (Context->Node)
	{
		FToolMenuSection& Section = Menu->AddSection("UnLive2DAnimBlueprintGraphNodePlay");
		Section.AddMenuEntry(FUnLive2DAnimBlueprintEditorCommands::Get().PlayNode);
	}
}

void UUnLive2DAnimBlueprintGraphNode_Root::CreateInputPins()
{
	CreatePin(EGPD_Input, TEXT("AnimBlueprintNode"), TEXT("Root"), NAME_None);
}

#undef LOCTEXT_NAMESPACE