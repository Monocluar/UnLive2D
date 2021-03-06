// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CubismConfig.h"
#include "Tickable.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "UnLive2D.generated.h"

class FUnLive2DRawModel;
class UMaterialInterface;
class UBodySetup;
class UTexture2D;
class UUnLive2DMotion;
class UUnLive2DExpression;


UCLASS(Blueprintable, BlueprintType, Category = UnLive2D, hidecategories=Object)
class UNLIVE2DASSET_API UUnLive2D : public UObject
{
    GENERATED_UCLASS_BODY()


public:
    // 动画频率
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Live2D , meta = (ClampMin = 0.0f))
        float PlayRate;

	// 贴图资源
	UPROPERTY(BlueprintReadWrite, EditAnywhere, AssetRegistrySearchable, Category = Live2D)
		TArray<TSoftObjectPtr<UTexture2D>> TextureAssets;

public:

	// 渲染设置数据
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Rendering)
		FModelConfig ModelConfigData;

	/** 此组件的着色颜色和不透明度 */
	UPROPERTY(EditAnywhere, Category = Rendering)
		FLinearColor TintColorAndOpacity;


public:

	// 更新UnLive2D其他参数
	FSimpleDelegate OnUpDataUnLive2DProperty;

protected:
	/** 动画播放完成 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Anim")
		void OnMotionPlayeEnd();

public:

	// 创建Live2D模型数据
	TSharedPtr<FUnLive2DRawModel> CreateLive2DRawModel() const;

#if WITH_EDITOR
	// 加载Live2D文件数据
	void LoadLive2DFileDataFormPath(const FString& InPath, TArray<FString>& TexturePaths, TArray<FUnLive2DMotionData>& LoadMotionData, TMap<FString, FUnLiveByteData>& LoadExpressionData);

#endif

    FORCEINLINE const FModelConfig& GetModelConfigData() { return ModelConfigData; }

	const FUnLive2DLoadData* GetUnLive2DLoadData();

	// 敲击
	void OnTap(const FVector2D& TapPosition);

	// 拖动
	void OnDrag(const FVector2D& DragPosition);

public:

	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif


protected:

	UPROPERTY()
		FUnLive2DLoadData Live2DFileData;

};