
#pragma once

#include "CoreMinimal.h"
#include "Math/RandomStream.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Animation/ActiveUnLive2DAnimBlueprint.h"
#include "UnLive2DAnimBlueprintNode_Base.generated.h"

#define DECLARE_UNLIVE2DANIMBLUEPRINT_ELEMENT(Type,Name)													\
	Type& Name = *((Type*)(Payload));																		\
	Payload += sizeof(Type);

#define DECLARE_UNLIVE2DANIMBLUEPRINT_ELEMENT_PTR(Type,Name)												\
	Type* Name = (Type*)(Payload);																			\
	Payload += sizeof(Type);


#define RETRIEVE_UNLIVE2DANIMBLUEPRINT_PAYLOAD( Size )														\
		uint8*	Payload					= NULL;																\
		uint32*	RequiresInitialization	= NULL;																\
		{																									\
			uint32* TempOffset = ActiveLive2DAnim.UnLive2DAnimNodeOffsetMap.Find(NodeAnimInstanceHash);		\
			uint32 Offset;																					\
			if( !TempOffset )																				\
			{																								\
				Offset = ActiveLive2DAnim.UnLive2DAnimNodeData.AddZeroed( Size + sizeof(uint32));			\
				ActiveLive2DAnim.UnLive2DAnimNodeOffsetMap.Add(NodeAnimInstanceHash, Offset);				\
				RequiresInitialization = (uint32*) &ActiveLive2DAnim.UnLive2DAnimNodeData[Offset];			\
				*RequiresInitialization = 1;																\
				Offset += sizeof(uint32);																	\
			}																								\
			else																							\
			{																								\
				RequiresInitialization = (uint32*) &ActiveLive2DAnim.UnLive2DAnimNodeData[*TempOffset];		\
				Offset = *TempOffset + sizeof(uint32);														\
			}																								\
			Payload = &ActiveLive2DAnim.UnLive2DAnimNodeData[Offset];										\
		}																									

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

	class UEdGraphNode* GetGraphNode() const;
#endif

#if WITH_EDITOR

	virtual void SetChildNodes(TArray<UUnLive2DAnimBlueprintNode_Base*>& InChildNodes);

	/** Get the name of a specific input pin */
	virtual FText GetInputPinName(int32 PinIndex) const { return FText::GetEmpty(); }

	virtual FText GetTitle() const { return GetClass()->GetDisplayNameText(); }

	/** Called as PIE begins */
	virtual void OnBeginPIE(const bool bIsSimulating) {};

	/** Called as PIE ends */
	virtual void OnEndPIE(const bool bIsSimulating) {};
#endif
	// 用于创建唯一字符串以标识唯一节点
	static UPTRINT GetNodeUnLive2DAnimInstanceHash(const UPTRINT ParentWaveInstanceHash, const UUnLive2DAnimBlueprintNode_Base* ChildNode, const uint32 ChildIndex);
	static UPTRINT GetNodeUnLive2DAnimInstanceHash(const UPTRINT ParentWaveInstanceHash, const UPTRINT ChildNodeHash, const uint32 ChildIndex);

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

	//由Live2D动画编辑器为允许子节点的节点调用。默认行为是连接单个接头。Dervied类可以重写，例如添加多个连接器。
	virtual void CreateStartingConnectors();
	virtual void InsertChildNode(int32 Index);
	virtual void RemoveChildNode(int32 Index);

	// 该动画节点播放时间
	virtual float GetDuration();

	// 返回节点中是否有延迟节点
	virtual bool HasDelayNode() const;

	// 返回声音是否具有sequencer节点。
	virtual bool HasConcatenatorNode() const;

	// 播放节点
	virtual void ParseNodes(FActiveUnLive2DAnimBlueprint& ActiveLive2DAnim, FUnLive2DAnimParseParameters& ParseParams, const UPTRINT NodeAnimInstanceHash);

	// 返回激活的节点
	virtual void GetAllNodes(TArray<UUnLive2DAnimBlueprintNode_Base*>& AnimBluepintNodes);

	// 该实例可以同时播放多少个动画
	virtual int32 GetNumSounds(const UPTRINT NodeAnimInstanceHash, FActiveUnLive2DAnimBlueprint& ActiveLive2DAnim) const;
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