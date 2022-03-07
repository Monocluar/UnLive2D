#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

#include "UnLive2DAnimBlueprintGeneratedClass.generated.h"

USTRUCT()
struct FUnLive2DStateMachineDebugData
{
	GENERATED_BODY()

public:
	FUnLive2DStateMachineDebugData()
		: MachineIndex(INDEX_NONE)
	{}

public:
	// 从状态节点映射到状态机中的状态条目
	TMap<TWeakObjectPtr<UEdGraphNode>, int32> NodeToStateIndex;
	TMap<TWeakObjectPtr<UEdGraphNode>, int32> NodeToTransitionIndex;

	//TWeakObjectPtr<UAnimGraphNode_StateMachineBase> MachineInstanceNode;

	// StateMaches数组中此计算机的索引
	int32 MachineIndex;
};

USTRUCT()
struct UNLIVE2DASSET_API FUnLive2DAnimBlueprintDebugData
{
	GENERATED_USTRUCT_BODY()

public:

	FUnLive2DAnimBlueprintDebugData()
#if WITH_EDITORONLY_DATA
#endif
	{
	}

#if WITH_EDITORONLY_DATA
public:

	// Map from state machine graphs to their corresponding debug data
	TMap<TWeakObjectPtr<const UEdGraph>, FUnLive2DStateMachineDebugData> StateMachineDebugData;

#endif
};


UCLASS()
class UNLIVE2DASSET_API UUnLive2DAnimBlueprintGeneratedClass : public UBlueprintGeneratedClass
{
	GENERATED_UCLASS_BODY()

public:

	// 此blueprint类中存在的状态机列表
	UPROPERTY()
		TArray<FBakedAnimationStateMachine> BakedAnimationStateMachines;

public:

#if WITH_EDITORONLY_DATA
	FUnLive2DAnimBlueprintDebugData AnimBlueprintDebugData;

	FUnLive2DAnimBlueprintDebugData& GetAnimBlueprintDebugData()
	{
		return AnimBlueprintDebugData;
	}

	const int32* GetNodePropertyIndexFromGuid(FGuid Guid, EPropertySearchMode::Type SearchMode = EPropertySearchMode::OnlyThis);

#endif
};