
#pragma once

#include "CoreMinimal.h"
#include "Math/RandomStream.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "UnLive2DAnimBlueprintNode_Base.generated.h"


UCLASS(Abstract, HideCategories=Object, editinlinenew)
class UNLIVE2DASSET_API UUnLive2DAnimBlueprintNode_Base : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	static const int32 MAX_ALLOWED_CHILD_NODES = 32;

	UPROPERTY()
		TArray<class UUnLive2DAnimBlueprintNode_Base*> ChildNodes;

#if WITH_EDITORONLY_DATA
	/** Node's Graph representation, used to get position. */
	UPROPERTY()
		UEdGraphNode* GraphNode;
#endif

#if WITH_EDITOR
	class UEdGraphNode* GetGraphNode() const;

	virtual void SetChildNodes(TArray<UUnLive2DAnimBlueprintNode_Base*>& InChildNodes);

	/** Get the name of a specific input pin */
	virtual FText GetInputPinName(int32 PinIndex) const { return FText::GetEmpty(); }

	virtual FText GetTitle() const { return GetClass()->GetDisplayNameText(); }

	/** Called as PIE begins */
	virtual void OnBeginPIE(const bool bIsSimulating) {};

	/** Called as PIE ends */
	virtual void OnEndPIE(const bool bIsSimulating) {};
#endif

public:
	
	// 返回此节点可能拥有的最大子节点数
	virtual int32 GetMaxChildNodes() const
	{
		return 1 ;
	}

	/** 返回此节点必须具有的最小子节点数 */
	virtual int32 GetMinChildNodes() const
	{
		return 0;
	}


protected:

	/** 此实例要使用的随机数流 of UUnLive2DAnimBlueprintNode_Base */
	FRandomStream RandomStream;

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostLoad() override;
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);
#endif //WITH_EDITOR
	virtual void Serialize(FArchive& Ar) override; 
	virtual bool CanBeClusterRoot() const override { return false; }
	virtual bool CanBeInCluster() const override { return false; }
};