
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/SoftObjectPtr.h"
#include "UnLive2DMotion.h"
#include "UnLive2DAnimBlueprintNode_AssetReferencer.h"
#include "UnLive2DAnimBlueprintNode_MotionPlayer.generated.h"

UCLASS(hideCategories =Object, editinlinenew, MinimalAPI, meta=(DisplayName="Motion Player"))
class UUnLive2DAnimBlueprintNode_MotionPlayer : public UUnLive2DAnimBlueprintNode_AssetReferencer
{
	GENERATED_UCLASS_BODY()

private:
	UPROPERTY(EditAnywhere, Category=Motion, meta=(DisplayName="Motion Asset"))
		TSoftObjectPtr<UUnLive2DMotion> UnLive2DMotionAssetPtr;

	UPROPERTY(transient)
		UUnLive2DMotion* UnLive2DMotion;

	uint8 bAsyncLoading:1;

public:
	// 播放速率
	UPROPERTY(EditAnywhere, Category=Motion)
		float PlayRate;

	// 动画达到末尾是否继续播放
	UPROPERTY(EditAnywhere, Category = Motion)
		bool bLooping;

public:

	UNLIVE2DASSET_API UUnLive2DMotion* GetUnLive2DMotion() const { return UnLive2DMotion; }
	UNLIVE2DASSET_API void SetUnLive2DMotion(UUnLive2DMotion* NewUnLive2DMotion);

protected:
	//~ Begin UObject Interface
		virtual void Serialize(FArchive & Ar) override;
#if WITH_EDITOR
		virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End UObject Interface

	//~ Begin UUnLive2DAnimBlueprintNode_Base Interface
	virtual int32 GetMaxChildNodes() const override;
	virtual float GetDuration() override;
#if WITH_EDITOR
	virtual FText GetTitle() const override;
#endif
	//~ End UUnLive2DAnimBlueprintNode_Base Interface


	//~ Begin UUnLive2DAnimBlueprintNode_AssetReferencer Interface
	virtual void LoadAsset(bool bAddToRoot = false) override;
	virtual void ClearAssetReferences() override;
	//~ End UUnLive2DAnimBlueprintNode_AssetReferencer Interface

	// 是否是异步加载
	bool IsCurrentlyAsyncLoadingAsset() const { return bAsyncLoading; }

private:

	void OnMotionAssetLoaded(const FName& PackageName, UPackage * Package, EAsyncLoadingResult::Type Result, bool bAddToRoot);
};