#include "AnimGraph/UnLive2DAnimGraphNode_Base.h"

UUnLive2DAnimGraphNode_Base::UUnLive2DAnimGraphNode_Base(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UUnLive2DAnimGraphNode_Base::OnNodeSelected(bool bInIsSelected, class FEditorModeTools& InModeTools, struct FUnLive2DAnimNode_Base* InRuntimeNode)
{

}

void UUnLive2DAnimGraphNode_Base::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
}

