#include "AnimBlueprintGraph/UnLive2DAnimBlueprintNode_Base.h"
#include "Misc/App.h"

UUnLive2DAnimBlueprintNode_Base::UUnLive2DAnimBlueprintNode_Base(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RandomStream.Initialize(FApp::bUseFixedSeed ? GetFName() : NAME_None);

}


#if WITH_EDITOR

class UEdGraphNode* UUnLive2DAnimBlueprintNode_Base::GetGraphNode() const
{
	return GraphNode;
}

void UUnLive2DAnimBlueprintNode_Base::SetChildNodes(TArray<UUnLive2DAnimBlueprintNode_Base*>& InChildNodes)
{
	int32 MaxChildNodes = GetMaxChildNodes();
	int32 MinChildNodes = GetMinChildNodes();
	if (MaxChildNodes >= InChildNodes.Num() && InChildNodes.Num() >= MinChildNodes)
	{
		ChildNodes = InChildNodes;
	}
}

void UUnLive2DAnimBlueprintNode_Base::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	MarkPackageDirty();
}

void UUnLive2DAnimBlueprintNode_Base::PostLoad()
{
	Super::PostLoad();
	// Make sure sound nodes are transactional (so they work with undo system)
	SetFlags(RF_Transactional);
}

void UUnLive2DAnimBlueprintNode_Base::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	UUnLive2DAnimBlueprintNode_Base* This = CastChecked<UUnLive2DAnimBlueprintNode_Base>(InThis);

	Collector.AddReferencedObject(This->GraphNode, This);

	Super::AddReferencedObjects(InThis, Collector);
}

#endif

void UUnLive2DAnimBlueprintNode_Base::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

#if WITH_EDITOR
	Ar << GraphNode;
#endif
}
