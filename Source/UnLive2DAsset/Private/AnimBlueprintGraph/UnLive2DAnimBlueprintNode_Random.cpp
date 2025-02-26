#include "AnimBlueprintGraph/UnLive2DAnimBlueprintNode_Random.h"

UUnLive2DAnimBlueprintNode_Random::UUnLive2DAnimBlueprintNode_Random(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NumRandomUsed = 0;
}

void UUnLive2DAnimBlueprintNode_Random::InsertChildNode(int32 Index)
{
	FixWeightsArray();
	FixHasBeenUsedArray();

	check(Index >= 0 && Index <= Weights.Num());
	check(ChildNodes.Num() == Weights.Num());

	Weights.InsertUninitialized(Index);
	Weights[Index] = 1.0f;

	HasBeenUsed.InsertUninitialized(Index);
	HasBeenUsed[Index] = false;

	Super::InsertChildNode(Index);
}

void UUnLive2DAnimBlueprintNode_Random::RemoveChildNode(int32 Index)
{
	FixWeightsArray();
	FixHasBeenUsedArray();

	check(Index >= 0 && Index < Weights.Num());
	check(ChildNodes.Num() == Weights.Num());

	Weights.RemoveAt(Index);
	HasBeenUsed.RemoveAt(Index);

	Super::RemoveChildNode(Index);
}

void UUnLive2DAnimBlueprintNode_Random::PostLoad()
{
	Super::PostLoad();

	if (!bShouldExcludeFromBranchCulling && !bAnimBlueprintExcludedFromBranchCulling)
	{
		int32 AmountOfBranchesToPreselect = DetermineAmountOfBranchesToPreselect();

		if (!GIsEditor && AmountOfBranchesToPreselect > 0)
		{
			int32 LastIndex = ChildNodes.Num() - 1;
			while (ChildNodes.Num() > AmountOfBranchesToPreselect)
			{
				RemoveChildNode(LastIndex--);
			}
		}
	}
#if WITH_EDITOR
	else if (GEditor != nullptr && (GEditor->bIsSimulatingInEditor || GEditor->PlayWorld != NULL))
	{
		UpdatePIEHiddenNodes();
	}
#endif

	FixWeightsArray();
	FixHasBeenUsedArray();
}

void UUnLive2DAnimBlueprintNode_Random::ParseNodes(FActiveUnLive2DAnimBlueprint& ActiveLive2DAnim, FUnLive2DAnimParseParameters& ParseParams, const UPTRINT NodeAnimInstanceHash)
{
	RETRIEVE_UNLIVE2DANIMBLUEPRINT_PAYLOAD(sizeof(int32));
	DECLARE_UNLIVE2DANIMBLUEPRINT_ELEMENT(int32, NodeIndex);


#if WITH_EDITOR
	bool bIsPIEAnimBlueprint = (GEditor != nullptr) && ((GEditor->bIsSimulatingInEditor || GEditor->PlayWorld != nullptr));
#endif //WITH_EDITOR

	// check to see if we have used up our random sounds
	if (bRandomizeWithoutReplacement && (HasBeenUsed.Num() > 0) && (NumRandomUsed >= HasBeenUsed.Num()
#if WITH_EDITOR
		|| (bIsPIEAnimBlueprint && NumRandomUsed >= (HasBeenUsed.Num() - PIEHiddenNodes.Num()))
#endif //WITH_EDITOR
		))
	{
		// reset all of the children nodes
		for (int32 i = 0; i < HasBeenUsed.Num(); ++i)
		{
			if (HasBeenUsed.Num() > NodeIndex)
			{
				HasBeenUsed[i] = false;
			}
		}
		// set the node that has JUST played to be true so we don't repeat it
		HasBeenUsed[NodeIndex] = true;
		NumRandomUsed = 1;
	}


	if (NodeIndex < ChildNodes.Num() && ChildNodes[NodeIndex])
	{
		ChildNodes[NodeIndex]->ParseNodes(ActiveLive2DAnim, ParseParams, NodeAnimInstanceHash);
	}
}

void UUnLive2DAnimBlueprintNode_Random::FixWeightsArray()
{
	if (Weights.Num() < ChildNodes.Num())
	{
		Weights.AddZeroed(ChildNodes.Num() - Weights.Num());
	}
	else if (Weights.Num() > ChildNodes.Num())
	{
		const int32 NumToRemove = Weights.Num() - ChildNodes.Num();
		Weights.RemoveAt(Weights.Num() - NumToRemove, NumToRemove);
	}
}

void UUnLive2DAnimBlueprintNode_Random::FixHasBeenUsedArray()
{
	// If HasBeenUsed and children got out of sync, we fix it first.
	if (HasBeenUsed.Num() < ChildNodes.Num())
	{
		HasBeenUsed.AddZeroed(ChildNodes.Num() - HasBeenUsed.Num());
	}
	else if (HasBeenUsed.Num() > ChildNodes.Num())
	{
		const int32 NumToRemove = HasBeenUsed.Num() - ChildNodes.Num();
		HasBeenUsed.RemoveAt(HasBeenUsed.Num() - NumToRemove, NumToRemove);
	}
}

#if WITH_EDITOR

void UUnLive2DAnimBlueprintNode_Random::SetChildNodes(TArray<UUnLive2DAnimBlueprintNode_Base*>& InChildNodes)
{
	Super::SetChildNodes(InChildNodes);

	if (Weights.Num() < ChildNodes.Num())
	{
		while (Weights.Num() < ChildNodes.Num())
		{
			int32 NewIndex = Weights.Num();
			Weights.InsertUninitialized(NewIndex);
			Weights[NewIndex] = 1.0f;
		}
	}
	else if (Weights.Num() > ChildNodes.Num())
	{
		const int32 NumToRemove = Weights.Num() - ChildNodes.Num();
		Weights.RemoveAt(Weights.Num() - NumToRemove, NumToRemove);
	}

	if (HasBeenUsed.Num() < ChildNodes.Num())
	{
		while (HasBeenUsed.Num() < ChildNodes.Num())
		{
			int32 NewIndex = HasBeenUsed.Num();
			HasBeenUsed.InsertUninitialized(NewIndex);
			HasBeenUsed[NewIndex] = false;
		}
	}
	else if (HasBeenUsed.Num() > ChildNodes.Num())
	{
		const int32 NumToRemove = HasBeenUsed.Num() - ChildNodes.Num();
		HasBeenUsed.RemoveAt(HasBeenUsed.Num() - NumToRemove, NumToRemove);
	}
}

void UUnLive2DAnimBlueprintNode_Random::OnBeginPIE(const bool bIsSimulating)
{
	UpdatePIEHiddenNodes();
}

void UUnLive2DAnimBlueprintNode_Random::UpdatePIEHiddenNodes()
{
	// 我们需要隐藏的节点
	int32 NodesToHide = ChildNodes.Num() - PreselectAtLevelLoad;
	if (PreselectAtLevelLoad > 0 && NodesToHide > 0)
	{
		PIEHiddenNodes.Empty();
		while (PIEHiddenNodes.Num() < NodesToHide)
		{
			PIEHiddenNodes.AddUnique(FMath::Rand() % ChildNodes.Num());
		}
		// 重置所有子节点和使用计数
		for (int32 i = 0; i < HasBeenUsed.Num(); ++i)
		{
			HasBeenUsed[i] = false;
		}
		NumRandomUsed = 0;
	}
	// 不隐藏零/负数量的节点
	else if ((PreselectAtLevelLoad <= 0 || NodesToHide <= 0))
	{
		PIEHiddenNodes.Empty();
	}
}
#endif

void UUnLive2DAnimBlueprintNode_Random::CreateStartingConnectors()
{
	// Random Sound Nodes default with two connectors.
	InsertChildNode(ChildNodes.Num());
	InsertChildNode(ChildNodes.Num());
}


int32 UUnLive2DAnimBlueprintNode_Random::DetermineAmountOfBranchesToPreselect()
{
	return PreselectAtLevelLoad;

}
