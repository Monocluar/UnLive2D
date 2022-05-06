#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Interfaces/Interface_AssetUserData.h"
#include "UnLive2DAnimBlueprint.generated.h"

class UUnLive2D;
class UEdGraph;
class UUnLive2DAnimBlueprintNode_Base;

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
class UNLIVE2DASSET_API UUnLive2DAnimBlueprint :  public UObject/*, public IInterface_AssetUserData*/
{
	GENERATED_UCLASS_BODY()


public:
	// 目标UnLive2D数据资源
	UPROPERTY(AssetRegistrySearchable, EditAnywhere, AdvancedDisplay, Category=ClassOptions)
		UUnLive2D* TargetUnLive2D;

	UPROPERTY()
		UUnLive2DAnimBlueprintNode_Base* FirstNode;

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
	/** Get the EdGraph of SoundNodes */
	UEdGraph* GetGraph();

	// 获取图表所有节点
	TArray<UUnLive2DAnimBlueprintNode_Base*>& GetGraphAllNodes();

	/** Sets the sound cue graph editor implementation.* */
	static void SeUnLive2DAnimBlueprintAnimEditor(TSharedPtr<IUnLive2DAnimBlueprintAnimEditor> InUnLive2DAnimBlueprintGraphEditor);

	/** Gets the sound cue graph editor implementation. */
	static TSharedPtr<IUnLive2DAnimBlueprintAnimEditor> GetUnLive2DAnimBlueprintAnimEditor();
#endif

protected:
#if WITH_EDITOR
	virtual void PostInitProperties() override;
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);

	void CreateGraph();

#endif
	virtual void PostLoad() override;
	virtual void Serialize(FStructuredArchive::FRecord Record) override;


private:
#if WITH_EDITOR

	static TSharedPtr<class IUnLive2DAnimBlueprintAnimEditor> UnLive2DAnimBlueprintAnimEditor;
#endif // WITH_EDITOR


	//virtual void Serialize(FStructuredArchive::FRecord Record) override;

};