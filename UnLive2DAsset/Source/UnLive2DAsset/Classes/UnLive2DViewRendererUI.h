
#pragma once


#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Components/Widget.h"
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

protected:

	virtual void SlateUpDataRender(TWeakPtr<class FUnLive2DRawModel> InUnLive2DRawModel);

	void InitUnLive2DRender();

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

	// UnLive2D 渲染模块
	TSharedPtr<class FUnLive2DRenderState> UnLive2DRenderPtr;

	friend class SUnLive2DViewUI;
};