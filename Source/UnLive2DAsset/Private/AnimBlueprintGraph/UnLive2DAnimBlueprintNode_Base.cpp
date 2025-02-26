#include "AnimBlueprintGraph/UnLive2DAnimBlueprintNode_Base.h"
#include "Misc/App.h"
#include "Animation/UnLive2DAnimBlueprint.h"

UUnLive2DAnimBlueprintNode_Base::UUnLive2DAnimBlueprintNode_Base(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RandomStream.Initialize(FApp::bUseFixedSeed ? GetFName() : NAME_None);

}

#if WITH_EDITORONLY_DATA
class UEdGraphNode* UUnLive2DAnimBlueprintNode_Base::GetGraphNode() const
{
	return GraphNode;
}
#endif

#if WITH_EDITOR
void UUnLive2DAnimBlueprintNode_Base::SetChildNodes(TArray<UUnLive2DAnimBlueprintNode_Base*>& InChildNodes)
{
	int32 MaxChildNodes = GetMaxChildNodes();
	int32 MinChildNodes = GetMinChildNodes();
	if (MaxChildNodes >= InChildNodes.Num() && InChildNodes.Num() >= MinChildNodes)
	{
		ChildNodes = InChildNodes;
	}
}
#endif

UPTRINT UUnLive2DAnimBlueprintNode_Base::GetNodeUnLive2DAnimInstanceHash(const UPTRINT ParentWaveInstanceHash, const UUnLive2DAnimBlueprintNode_Base* ChildNode, const uint32 ChildIndex)
{
	return GetNodeUnLive2DAnimInstanceHash(ParentWaveInstanceHash, reinterpret_cast<const UPTRINT>(ChildNode), ChildIndex);
}

UPTRINT UUnLive2DAnimBlueprintNode_Base::GetNodeUnLive2DAnimInstanceHash(const UPTRINT ParentWaveInstanceHash, const UPTRINT ChildNodeHash, const uint32 ChildIndex)
{
	const uint32 ChildHash = PointerHash(reinterpret_cast<const void*>(ChildNodeHash), GetTypeHash(ChildIndex));
	const uint32 Hash = PointerHash(reinterpret_cast<const void*>(ParentWaveInstanceHash), ChildHash);

	return static_cast<UPTRINT>(Hash);
}

void UUnLive2DAnimBlueprintNode_Base::CreateStartingConnectors()
{
	int32 ConnectorsToMake = FMath::Max(1, GetMinChildNodes());
	while (ConnectorsToMake > 0)
	{
		InsertChildNode(ChildNodes.Num());
		--ConnectorsToMake;
	}
}

void UUnLive2DAnimBlueprintNode_Base::InsertChildNode(int32 Index)
{
	check( Index >= 0 && Index <= ChildNodes.Num() );
	int32 MaxChildNodes = GetMaxChildNodes();
	if (MaxChildNodes > ChildNodes.Num())
	{
		ChildNodes.InsertZeroed(Index);
#if WITH_EDITOR
		UUnLive2DAnimBlueprint::GetUnLive2DAnimBlueprintAnimEditor()->CreateInputPin(GetGraphNode());
#endif //WITH_EDITORONLY_DATA
	}
}

void UUnLive2DAnimBlueprintNode_Base::RemoveChildNode(int32 Index)
{
	check(Index >= 0 && Index < ChildNodes.Num());
	int32 MinChildNodes = GetMinChildNodes();
	if (ChildNodes.Num() > MinChildNodes)
	{
		ChildNodes.RemoveAt(Index);
	}
}

float UUnLive2DAnimBlueprintNode_Base::GetDuration()
{
	float MaxDuration = 0.f;
	for (UUnLive2DAnimBlueprintNode_Base* ChildNode : ChildNodes)
	{
		if (ChildNode)
		{
			ChildNode->ConditionalPostLoad();
			MaxDuration = FMath::Max(ChildNode->GetDuration(), MaxDuration);
		}
	}

	return MaxDuration;
}

bool UUnLive2DAnimBlueprintNode_Base::HasDelayNode() const
{
	for (UUnLive2DAnimBlueprintNode_Base* ChildNode : ChildNodes)
	{
		if (ChildNode == nullptr) continue;

		ChildNode->ConditionalPostLoad();
		if (ChildNode->HasDelayNode())
		{
			return true;
		}
	}

	return false;
}

bool UUnLive2DAnimBlueprintNode_Base::HasConcatenatorNode() const
{
	for (UUnLive2DAnimBlueprintNode_Base* ChildNode : ChildNodes)
	{
		if (ChildNode == nullptr) continue;

		ChildNode->ConditionalPostLoad();
		if (ChildNode->HasConcatenatorNode())
		{
			return true;
		}
	}

	return false;
}

void UUnLive2DAnimBlueprintNode_Base::ParseNodes(FActiveUnLive2DAnimBlueprint& ActiveLive2DAnim,  FUnLive2DAnimParseParameters& ParseParams, const UPTRINT NodeAnimInstanceHash)
{
	for (int32 i = 0; i < ChildNodes.Num() && i < GetMaxChildNodes(); ++i)
	{
		if (ChildNodes[i])
		{
			ChildNodes[i]->ParseNodes(ActiveLive2DAnim, ParseParams, GetNodeUnLive2DAnimInstanceHash(NodeAnimInstanceHash, ChildNodes[i], i));
		}
	}
}

void UUnLive2DAnimBlueprintNode_Base::GetAllNodes(TArray<UUnLive2DAnimBlueprintNode_Base*>& AnimBluepintNodes)
{
	AnimBluepintNodes.Add(this);
	for (UUnLive2DAnimBlueprintNode_Base* ChildNode : ChildNodes)
	{
		if (ChildNode == nullptr) continue;

		ChildNode->GetAllNodes(AnimBluepintNodes);
	}
}

int32 UUnLive2DAnimBlueprintNode_Base::GetNumSounds(const UPTRINT NodeAnimInstanceHash, FActiveUnLive2DAnimBlueprint& ActiveLive2DAnim) const
{
	int32 NumSounds = 0;
	for (int32 i = 0; i < ChildNodes.Num(); ++i)
	{
		if (ChildNodes[i] == nullptr) continue;
		const UPTRINT ChildNodeWaveInstanceHash = GetNodeUnLive2DAnimInstanceHash(NodeAnimInstanceHash, ChildNodes[i], i);
		NumSounds += ChildNodes[i]->GetNumSounds(NodeAnimInstanceHash, ActiveLive2DAnim);
	}

	return NumSounds;
}

#if WITH_EDITOR
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
