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


UCLASS(Blueprintable, BlueprintType, Category = UnLive2D)
class UNLIVE2DASSET_API UUnLive2D : public UObject
{
    GENERATED_UCLASS_BODY()


public:
    // 动画频率
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Live2D , meta = (ClampMin = 0.0f))
        float PlayRate;

	/** 显示四边形大小. */
	UPROPERTY(EditAnywhere, Category = Live2D)
		FIntPoint DrawSize;

	// 相对于位置放置的对齐/轴心点。
	UPROPERTY(EditAnywhere, Category = Live2D)
		FVector2D Pivot;

	// Live2D动画组
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = Live2D)
		TArray<FName> Live2DMotionGroup;

	// 播放动画
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Live2D)
		int32 PlayMotionIndex = 0;

public:
	// Collision domain (no collision, 2D, or 3D)
	UPROPERTY(Category = Collision, EditAnywhere)
		TEnumAsByte<EUnLive2DCollisionMode::Type> UnLive2DCollisionDomain;

public:

	// 自定义碰撞几何体多边形（在纹理空间中）
	UPROPERTY(Category = Collision, EditAnywhere)
		FUnLive2DGeometryCollection CollisionGeometry;

public:

	// UnLive2D组件的材质实例
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Rendering)
		UMaterialInterface* UnLive2DMaterial;

	// 渲染设置数据
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Rendering)
		FModelConfig ModelConfigData;

	/** 此组件的着色颜色和不透明度 */
	UPROPERTY(EditAnywhere, Category = Rendering)
		FLinearColor TintColorAndOpacity;

	// 组件的背景色
	UPROPERTY(EditAnywhere, Category = Rendering)
		FLinearColor BackgroundColor;

	// 从窗口的Live2D纹理设置向视口渲染半透明或遮罩UI时要使用的不透明度（0.0-1.0）
	UPROPERTY(EditAnywhere, Category = Rendering, meta = (ClampMin = 0.0f, ClampMax = 1.0f))
		float OpacityFromTexture;


public:
    // UnLive2D更新
    FSimpleDelegate OnUpDataUnLive2D;

	// 更新UnLive2D物体大小
	FSimpleDelegate OnUpDataUnLive2DBodySetup;

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
	void LoadLive2DFileDataFormPath(const FString& InPath);
#endif

    FORCEINLINE TWeakPtr<FUnLive2DRawModel> GetUnLive2DRawModel() {return UnLive2DRawModel; }

    FORCEINLINE const FModelConfig& GetModelConfigData() {return ModelConfigData; }

	const FUnLive2DLoadData* GetUnLive2DLoadData();

	// 中心点位置
	FVector2D GetRawPivotPosition() const;


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

	// <创建Polygon Box碰撞
	void CreatePolygonFromBoundingBox(FUnLive2DGeometryCollection& GeomOwner, bool bUseTightBounds);

	// 计算矩形，找到仍然包含alpha高于AlphaThreshold的所有像素的最紧边界。
	bool FindTextureBoundingBox(/*out*/ FVector2D& OutBoxPosition, /*out*/ FVector2D& OutBoxSize);

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