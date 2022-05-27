
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "CubismConfig.h"
#include "UnLive2DMotion.generated.h"

class UUnLive2D;

UCLASS(Blueprintable, BlueprintType, Category = UnLive2DMotion, hidecategories=Object)
class UNLIVE2DASSET_API UUnLive2DMotion : public UObject,  public IInterface_AssetUserData
{
	GENERATED_BODY()

public:
	// UnLive2D设置
	UPROPERTY(AssetRegistrySearchable, Category = Animation, VisibleAnywhere)
		UUnLive2D* UnLive2D;

	// 正常速率播放一次动画需要时间
	UPROPERTY(Category = Developer, AssetRegistrySearchable, VisibleAnywhere, BlueprintReadOnly)
		float Duration;


	// 与资产一起存储的用户数据数组
	UPROPERTY(EditAnywhere, AdvancedDisplay, Instanced, Category = Live2D)
		TArray<UAssetUserData*> AssetUserData;
public:

#if WITH_EDITOR
	bool LoadLive2DMotionData(const FString& ReadMotionPath, EUnLive2DMotionGroup InMotionGroupType = EUnLive2DMotionGroup::None, int32 InMotionCount = 0, float FadeInTime = 0.5f, float FadeOutTime = 0.5f);

	void SetLive2DMotionData(FUnLive2DMotionData& InMotionData);
#endif

	const FUnLive2DMotionData* GetMotionData();


protected:
	// 动作数据
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, AssetRegistrySearchable, Category = Animation)
		FUnLive2DMotionData MotionData;

protected:

	//~ Begin IInterface_AssetUserData Interface
	virtual void AddAssetUserData(UAssetUserData* InUserData) override;
	virtual void RemoveUserDataOfClass(TSubclassOf<UAssetUserData> InUserDataClass) override;
	virtual UAssetUserData* GetAssetUserDataOfClass(TSubclassOf<UAssetUserData> InUserDataClass) override;
	virtual const TArray<UAssetUserData*>* GetAssetUserDataArray() const override;
	//~ End IInterface_AssetUserData Interface

};