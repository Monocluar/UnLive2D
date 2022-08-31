
#pragma once

#include "CoreMinimal.h"
#include "CubismConfig.h"
#include "UnLive2DAnimBase.h"
#include "UnLive2DMotion.generated.h"


DECLARE_DELEGATE_TwoParams(FOnUnLive2DMotionEnded, class UUnLive2DMotion*, bool /*bInterrupted*/)

UCLASS(Blueprintable, BlueprintType, Category = UnLive2DAnim, hidecategories=Object)
class UNLIVE2DASSET_API UUnLive2DMotion : public UUnLive2DAnimBase
{
	GENERATED_BODY()

public:
	UUnLive2DMotion();

public:
	// 是否循环播放
	UPROPERTY(EditAnywhere, Category = Animation, AssetRegistrySearchable)
		uint8 bLooping : 1;

	// 正常速率播放一次动画需要时间
	UPROPERTY(Category = Developer, AssetRegistrySearchable, VisibleAnywhere, BlueprintReadOnly)
		float Duration;

public:
	
	FOnUnLive2DMotionEnded OnUnLive2DMotionEnded;

public:

#if WITH_EDITOR
	bool LoadLive2DMotionData(const FString& ReadMotionPath, EUnLive2DMotionGroup InMotionGroupType = EUnLive2DMotionGroup::None, int32 InMotionCount = 0, float FadeInTime = 0.5f, float FadeOutTime = 0.5f);

	void SetLive2DMotionData(FUnLive2DMotionData& InMotionData);
#endif

	const FUnLive2DMotionData* GetMotionData();

	void OnPlayAnimEnd();

	void OnPlayAnimInterrupted();

protected:
	// 动作数据
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, AssetRegistrySearchable, Category = Animation)
		FUnLive2DMotionData MotionData;

public:

	virtual void Parse(struct FActiveUnLive2DAnimBlueprint& ActiveLive2DAnim, struct FUnLive2DAnimParseParameters& ParseParams, const UPTRINT NodeAnimInstanceHash) override;

private:

	struct FUnLive2DAnimInstance& HandleStart(struct FActiveUnLive2DAnimBlueprint& ActiveLive2DAnim, const UPTRINT NodeAnimInstanceHash) const;

private:

	bool bFinished;
};