
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "UnLive2D.h"
#include "UnLive2DAnimBase.generated.h"


UCLASS(hideCategories=Object, Abstract, editinlinenew, BlueprintType)
class UNLIVE2DASSET_API UUnLive2DAnimBase : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	// UnLive2D设置
	UPROPERTY(AssetRegistrySearchable, Category = Animation, VisibleAnywhere)
		UUnLive2D* UnLive2D;

public:	

	virtual void Parse(struct FActiveUnLive2DAnimBlueprint& ActiveLive2DAnim, struct FUnLive2DAnimParseParameters& ParseParams, const UPTRINT NodeAnimInstanceHash){};

#if WITH_EDITOR
public:
	// 获取UnLive2D动画参数组
	virtual bool GetAnimParamterGroup(TWeakObjectPtr<class UUnLive2DRendererComponent>& ObsComp, TArray<struct FUnLive2DParameterData_Expression>& ParameterArr){ return false; }

	// 设置UnLive2D动画参数值
	virtual void SetAnimParamterValue(FName ParameterStr, float NewParameter){};

	// 设置UnLive2D动画叠加类型
	virtual void SetAnimParamterBlendType(FName ParameterStr, float NewParameter) {};
#endif
};