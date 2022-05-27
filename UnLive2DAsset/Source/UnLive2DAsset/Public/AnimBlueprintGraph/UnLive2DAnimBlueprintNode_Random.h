
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UnLive2DAnimBlueprintNode_Base.h"
#include "UnLive2DAnimBlueprintNode_Random.generated.h"


UCLASS(hideCategories=Object, editinlinenew, MinimalAPI, meta=(DisplayName="Random"))
class UUnLive2DAnimBlueprintNode_Random : public UUnLive2DAnimBlueprintNode_Base
{
	GENERATED_UCLASS_BODY()

public:

	UPROPERTY(EditAnywhere, editfixedsize, Category = Random)
		TArray<float> Weights;

	// 跟踪那些动画节点播放
	UPROPERTY(transient)
		TArray<bool> HasBeenUsed;

	UPROPERTY(transient)
		int32 NumRandomUsed;

	// 如果大于0，则在每个水平负载上，将随机选择这样数量的输入
	UPROPERTY(EditAnywhere, Category = Random)
		int32 PreselectAtLevelLoad;

	// 如果设置为true，则对于预加载随机分支最多的平台，不会在加载时剔除此随机节点
	UPROPERTY(EditAnywhere, Category = Random)
		uint8 bShouldExcludeFromBranchCulling : 1;

	// 从分支剔除中排除
	UPROPERTY()
		uint8 bAnimBlueprintExcludedFromBranchCulling : 1;

#if WITH_EDITORONLY_DATA
	/** 仅编辑器隐藏以复制PreselectAtLevelLoad行为的节点列表 */
	UPROPERTY(transient)
		TArray<int32> PIEHiddenNodes;
#endif //WITH_EDITORONLY_DATA

protected:
	//~ Begin UUnLive2DAnimBlueprintNode_Base Interface.
	virtual int32 GetMaxChildNodes() const override
	{
		return MAX_ALLOWED_CHILD_NODES;
	}
	virtual void InsertChildNode(int32 Index) override;
	virtual void RemoveChildNode(int32 Index) override;

	//~ Begin UObject Interface
	virtual void PostLoad() override;
	//~ End UObject Interface

	// 重新设置权重数组
	void FixWeightsArray();
	// 修复已使用数组
	void FixHasBeenUsedArray();
protected:
#if WITH_EDITOR
	virtual void SetChildNodes(TArray<UUnLive2DAnimBlueprintNode_Base*>& InChildNodes) override;
	virtual void OnBeginPIE(const bool bIsSimulating) override;
	void UpdatePIEHiddenNodes();
#endif //WITH_EDITOR
	virtual void CreateStartingConnectors() override;

private:
	// 确定要在此随机节点上使用的分支数量，以便我们可以剔除其余的分支。
	// 不剔除任何分支则输出为0
	int32 DetermineAmountOfBranchesToPreselect();
};