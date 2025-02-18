
#pragma once


#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Components/Widget.h"
#include "CubismConfig.h"
#include "UnLive2DViewRendererUI.generated.h"

class SUnLive2DViewUI;
class UUnLive2D;
class UUnLive2DMotion;
class UUnLive2DExpression;

UCLASS()
class UNLIVE2DASSET_API UUnLive2DViewRendererUI : public UWidget
{
	GENERATED_UCLASS_BODY()

protected:
	// UnLive2D资源
	UPROPERTY(EditAnywhere, Category = Live2D)
		UUnLive2D* SourceUnLive2D;

	// 动画频率
	UPROPERTY(EditAnywhere, Category = Live2D, meta = (ClampMin = 0.0f))
		float PlayRate;

	UPROPERTY()
	FGetFloat PlayRateDelegate;

public:
	// 渲染模式
	UPROPERTY(EditAnywhere, Category = Rendering)
		EUnLive2DRenderType UnLive2DRenderType;


public: // Mesh
	// Live2D颜色混合模式为CubismBlendMode_Normal使用的材质
	UPROPERTY(EditAnywhere, Category = Rendering, meta = (AllowedClasses = "MaterialInterface", EditCondition = "UnLive2DRenderType == EUnLive2DRenderType::Mesh", EditConditionHides))
		FSoftObjectPath UnLive2DNormalMaterial;

	// Live2D颜色混合模式为CubismBlendMode_Additive使用的材质
	UPROPERTY(EditAnywhere, Category = Rendering, meta = (AllowedClasses = "MaterialInterface", EditCondition = "UnLive2DRenderType == EUnLive2DRenderType::Mesh", EditConditionHides))
		FSoftObjectPath UnLive2DAdditiveMaterial;

	// Live2D颜色混合模式为CubismBlendMode_Multiplicative使用的材质
	UPROPERTY(EditAnywhere, Category = Rendering, meta = (AllowedClasses = "MaterialInterface", EditCondition = "UnLive2DRenderType == EUnLive2DRenderType::Mesh", EditConditionHides))
		FSoftObjectPath UnLive2DMultiplyMaterial;

public: // RenderTarget

	// 渲染大小
	UPROPERTY(EditAnywhere, Category = Rendering, meta = (UIMin = "256", UIMax = "2048", EditCondition = "UnLive2DRenderType == EUnLive2DRenderType::RenderTarget", EditConditionHides))
		int32 RenderTargetSize;

	UPROPERTY(EditAnywhere, Category = Rendering, meta = (AllowedClasses = "MaterialInterface", EditCondition = "UnLive2DRenderType == EUnLive2DRenderType::RenderTarget", EditConditionHides))
		FSoftObjectPath UnLive2DRTMaterial;

public:
	// 播放动画
	//UFUNCTION(BlueprintCallable, Category = "Anim")
	virtual void  PlayMotion(UUnLive2DMotion* InMotion);

	// 播放表情
	UFUNCTION(BlueprintCallable, Category = "Anim")
		virtual void PlayExpression(UUnLive2DExpression* InExpression);

	// 停止播放动画
	UFUNCTION(BlueprintCallable, Category = "Anim")
		virtual void StopMotion();

public:

	FORCEINLINE const UUnLive2D* GetUnLive2D() const { return SourceUnLive2D; };



#if WITH_EDITOR
protected:
	virtual const FText GetPaletteCategory() override;
#endif

#if	WITH_ACCESSIBILITY
	virtual TSharedPtr<SWidget> GetAccessibleWidget() const override;
#endif

protected:

	virtual TSharedRef<SWidget> RebuildWidget() override;

	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

protected:

	TSharedPtr<SUnLive2DViewUI> MySlateWidget;

protected:
	PROPERTY_BINDING_IMPLEMENTATION(float, PlayRate);
};