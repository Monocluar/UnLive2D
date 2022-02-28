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


UCLASS(Blueprintable, BlueprintType, Category = UnLive2D)
class UNLIVE2DASSET_API UUnLive2D : public UObject
{
    GENERATED_UCLASS_BODY()


public:
    // 动画频率
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Live2D , meta = (ClampMin = 0.0f))
        float PlayRate;

	// Live2D动画组
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = Live2D)
		TArray<FName> Live2DMotionGroup;

	// 播放动画
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Live2D)
		int32 PlayMotionIndex = 0;

	// 贴图资源
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Live2D)
		TArray<TSoftObjectPtr<UTexture2D>> TextureAssets;

public:
	// Live2D颜色混合模式为CubismBlendMode_Normal使用的材质
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Rendering)
		UMaterialInterface* UnLive2DNormalMaterial;

	// Live2D颜色混合模式为CubismBlendMode_Additive使用的材质
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Rendering)
		UMaterialInterface* UnLive2DAdditiveMaterial;

	// Live2D颜色混合模式为CubismBlendMode_Multiplicative使用的材质
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Rendering)
		UMaterialInterface* UnLive2DMultiplyMaterial;

	// 材质读取图片参数名
	UPROPERTY(Category = Rendering, EditAnywhere, BlueprintReadWrite)
		FName TextureParameterName;

public:

	// 渲染设置数据
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Rendering)
		FModelConfig ModelConfigData;

	/** 此组件的着色颜色和不透明度 */
	UPROPERTY(EditAnywhere, Category = Rendering)
		FLinearColor TintColorAndOpacity;

public:
    // UnLive2D更新
    FSimpleDelegate OnUpDataUnLive2D;

	// 更新UnLive2D其他参数
	FSimpleDelegate OnUpDataUnLive2DProperty;

protected:
	/** 动画播放完成 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void OnMotionPlayeEnd();

public:
    // 加载Live2D模型
    void InitLive2D();

#if WITH_EDITOR
	// 加载Live2D文件数据
	void LoadLive2DFileDataFormPath(const FString& InPath, TArray<FString>& TexturePaths, TArray<FUnLive2DMotionData>& LoadMotionData);
#endif

    FORCEINLINE TWeakPtr<FUnLive2DRawModel> GetUnLive2DRawModel() {return UnLive2DRawModel; }

    FORCEINLINE const FModelConfig& GetModelConfigData() {return ModelConfigData; }

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


public:

	void SetOwnerObject(UObject* Owner);

private:
    // Live2D模型设置模块
    TSharedPtr<FUnLive2DRawModel> UnLive2DRawModel;

	// 附带该数据的渲染类（UnLive2DComponent 或其他）
	TWeakObjectPtr<UObject> OwnerObject;

protected:

	UPROPERTY()
		FUnLive2DLoadData Live2DFileData;

};