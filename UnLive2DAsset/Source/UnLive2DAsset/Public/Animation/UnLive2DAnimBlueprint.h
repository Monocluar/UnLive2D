#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Interfaces/Interface_AssetUserData.h"
#include "UnLive2DAnimBlueprint.generated.h"

class UUnLive2D;
class UEdGraph;
class UUnLive2DAnimBlueprintNode_Base;
class UAssetUserData;

#if WITH_EDITOR
class IUnLive2DAnimBlueprintAnimEditor
{
public:
	virtual ~IUnLive2DAnimBlueprintAnimEditor(){}

	// 创建新的UnLive2D动画图组
	virtual UEdGraph* CreateUnLive2DAnimBlueprintGraph(UUnLive2DAnimBlueprint* InAnimBlueprint) = 0;

	// 删除空节点
	virtual void RemoveNullNodes(UUnLive2DAnimBlueprint* InAnimBlueprint) = 0;

	// 给指定图表创建新的输入Pin
	virtual void CreateInputPin(UEdGraphNode* AnimBlueprintNode) = 0;

	// 设置动画蓝图节点
	virtual void SetupAnimBlueprintNode(UEdGraph* AnimBlueprintGraph, UUnLive2DAnimBlueprintNode_Base* InAnimBlueprintNode, bool bSelectNewNode) = 0;

	/** 链接动画节点的图形节点。 */
	virtual void LinkGraphNodesFromAnimBlueprintNodes(UUnLive2DAnimBlueprint* AnimBlueprint) = 0;

	/** 从图形节点编译动画节点. */
	virtual void CompileAnimBlueprintNodesFromGraphNodes(UUnLive2DAnimBlueprint* AnimBlueprint) = 0;

	/** 重命名动画提示节点中的所有管脚 */
	virtual void RenameNodePins(UUnLive2DAnimBlueprintNode_Base* AnimBlueprintNode) = 0;
};

#endif

UCLASS(hidecategories=object, Blueprintable, BlueprintType)
class UNLIVE2DASSET_API UUnLive2DAnimBlueprint :  public UObject, public IInterface_AssetUserData
{
	GENERATED_UCLASS_BODY()

public:
	// 目标UnLive2D数据资源
	UPROPERTY(AssetRegistrySearchable, EditAnywhere, AdvancedDisplay, Category=ClassOptions)
		UUnLive2D* TargetUnLive2D;

	UPROPERTY()
		UUnLive2DAnimBlueprintNode_Base* FirstNode;

	// 与资产一起存储的用户数据数组
	UPROPERTY(EditAnywhere, AdvancedDisplay, Instanced, Category = Live2D)
		TArray<UAssetUserData*> AssetUserData;

	/** 动画持续时间 */
	UPROPERTY(Category = Developer, AssetRegistrySearchable, VisibleAnywhere, BlueprintReadOnly)
		float Duration;

	// 是否有延迟节点
	UPROPERTY()
		uint8 bHasDelayNode : 1;

	// 是否有具有sequencer节点
	UPROPERTY()
		uint8 bHasConcatenatorNode : 1;

	// 出于内存目的，忽略每个平台的随机节点剔除
	UPROPERTY(EditAnywhere, Category = Memory, Meta = (DisplayName = "Disable Random Branch Culling"))
		uint8 bExcludeFromRandomNodeBranchCulling : 1;
protected:
#if WITH_EDITORONLY_DATA

	UPROPERTY()
		TArray<UUnLive2DAnimBlueprintNode_Base*> AllNodes;

	UPROPERTY()
		UEdGraph* UnLive2DAnimBlueprintGraph;

#endif

protected:
	void CacheAggregateValues();

private:
	void EvaluateNodes(bool bAddToRoot);

public:
#if WITH_EDITOR

	// 获取图表所有节点
	TArray<UUnLive2DAnimBlueprintNode_Base*>& GetGraphAllNodes();
#endif

protected:
#if WITH_EDITOR
	virtual void PostInitProperties() override;
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);

	// 创建图表
	void CreateGraph();

	// 清空图表中所有节点
	void ClearGraph();

	// 设置动画节点在图表
	void SetupAnimNode(UUnLive2DAnimBlueprintNode_Base* InAnimNode, bool bSelectNewNode = true);

	// 连接动画节点在图表中
	void LinkGraphNodesFromUnLive2DAnimNodes();


public:

	/** 获取动画图表 */
	UEdGraph* GetGraph();

	/** 重置所有图表数据和节点 */
	void ResetGraph();

	// 设置动画节点图表编辑器实现
	static void SeUnLive2DAnimBlueprintAnimEditor(TSharedPtr<IUnLive2DAnimBlueprintAnimEditor> InUnLive2DAnimBlueprintGraphEditor);

	/** 获取动画节点图表编辑器实现. */
	static TSharedPtr<IUnLive2DAnimBlueprintAnimEditor> GetUnLive2DAnimBlueprintAnimEditor();

	// 编辑在图表中的节点
	void CompileUnLive2DAnimNodesFromGraphNodes();

	/**在此提示中构造并初始化节点 */
	template<class T>
	T* ConstructAnimNode(TSubclassOf<UUnLive2DAnimBlueprintNode_Base> AnimBlueprintNodeClass = T::StaticClass(), bool bSelectNewNode = true)
	{
		// Set flag to be transactional so it registers with undo system
		T* AnimBlueprintNode = NewObject<T>(this, AnimBlueprintNodeClass, NAME_None, RF_Transactional);
#if WITH_EDITOR
		AllNodes.Add(AnimBlueprintNode);
		SetupAnimNode(AnimBlueprintNode, bSelectNewNode);
#endif // WITH_EDITORONLY_DATA
		return AnimBlueprintNode;
	}
private:

	/** 递归地在此动画节点中的随机节点上设置分支消隐排除。 */
	void RecursivelySetExcludeBranchCulling(UUnLive2DAnimBlueprintNode_Base* CurrentNode);

#endif
	virtual void PostLoad() override;
	virtual void Serialize(FStructuredArchive::FRecord Record) override;
	virtual bool CanBeClusterRoot() const override { return false; };
	virtual bool CanBeInCluster() const override{ return false; };

protected:
	//~ Begin IInterface_AssetUserData Interface
	virtual void AddAssetUserData(UAssetUserData* InUserData) override;
	virtual void RemoveUserDataOfClass(TSubclassOf<UAssetUserData> InUserDataClass) override;
	virtual UAssetUserData* GetAssetUserDataOfClass(TSubclassOf<UAssetUserData> InUserDataClass) override;
	virtual const TArray<UAssetUserData*>* GetAssetUserDataArray() const override;
	//~ End IInterface_AssetUserData Interface

private:
#if WITH_EDITOR

	static TSharedPtr<class IUnLive2DAnimBlueprintAnimEditor> UnLive2DAnimBlueprintAnimEditor;
#endif // WITH_EDITOR


	//virtual void Serialize(FStructuredArchive::FRecord Record) override;

};